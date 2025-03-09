#include <unordered_set>
#include <functional>
#include <wfrest/Json.h>
#include "../shared/tinyxml2.h"
#include "../shared/Mylogger.h"
#include "../shared/Configuration.h"
#include "../shared/SplitTool.h"
#include "PageLibPreprocessor.h"
using namespace std::placeholders;
using namespace tinyxml2;
using namespace simhash;
using std::unique_ptr;

const size_t topN = 10;
const size_t docNum = 100;

struct MyCompare{
    bool operator()(uint64_t lhs, uint64_t rhs)const{
        return Simhasher::isEqual(lhs,rhs,3);
    }
};


PageLibPreprocessor::PageLibPreprocessor(Configuration *pconf, SplitTool *ptool)
    :_pconf(pconf)
     ,_pWordCutter(ptool)
     ,_pSimhasher(createSimhasher())
{}

Simhasher * PageLibPreprocessor::createSimhasher(){
    auto configs = _pconf->getConfigMap();
    const char* const DICT_PATH = configs["DICT_PATH"].c_str();
    const char* const HMM_PATH = configs["HMM_PATH"].c_str();
    const char* const USER_DICT_PATH = configs["USER_DICT_PATH"].c_str();
    const char* const IDF_PATH = configs["IDF_PATH"].c_str();
    const char* const STOP_WORD_PATH = configs["STOP_WORD_PATH"].c_str();

    //return new Simhasher(DICT_PATH,HMM_PATH,IDF_PATH,STOP_WORD_PATH);
    return new Simhasher(DICT_PATH,HMM_PATH,USER_DICT_PATH,IDF_PATH,STOP_WORD_PATH);
}

void PageLibPreprocessor::cutRedundantPage(){
    auto configs = _pconf->getConfigMap();
    string webpageLib_old = configs["webpageLib_old"];
    string offsetLib_old = configs["offsetLib_old"];

    ifstream ifs(offsetLib_old);
    if(!ifs){
        LogError("open offsetLib_old file failed");
        return;
    }
    while(ifs){
        size_t docId;
        long pos,len;
        ifs >> docId >> pos >> len;
        _offsetLib.emplace_back(pos,len);
    }
    ifs.close();

    ifs.open(webpageLib_old);
    if(!ifs){
        LogError("open webpageLib_old failed");
        return;
    }
    unordered_set<uint64_t,std::hash<uint64_t>,MyCompare> hashs;
    //分批处理所有文章，每次读取20篇，进行处理
    for(size_t i = 0 ; i < _offsetLib.size(); i+=docNum){
        //计算20篇文章的大小，并读取
        ifs.seekg(_offsetLib[i].first);
        size_t docsSize = 0;
        for(size_t j = 0; j < docNum && i+j < _offsetLib.size() - 1; j++){
            docsSize += _offsetLib[i + j].second;
        }
        char buf[docsSize + 1];
        buf[docsSize] = 0;
        ifs.read(buf,docsSize);

        //用XML处理当前批次的doc
        XMLDocument doc;
        doc.Parse(buf);
        if(doc.ErrorID()){
            LogError("XML Parse failed");
            cout << doc.ErrorStr() << "\n";
            return;
        }
        XMLElement * pItemNode = doc.FirstChildElement("doc");
        while(pItemNode){
            string title,link,content;
            XMLElement *temp = pItemNode->FirstChildElement("content");
            //跳过坏数据
            if(temp == nullptr){
                pItemNode = pItemNode->NextSiblingElement("doc");
                continue;
            }
            if(temp->GetText() == nullptr){
                pItemNode = pItemNode->NextSiblingElement("doc");
                continue;
            }
            content = temp->GetText();

            temp = pItemNode->FirstChildElement("title");
            if(temp != nullptr){
                title = temp->GetText();
            }
            temp = pItemNode->FirstChildElement("link");
            if(temp != nullptr){
                link = temp->GetText();
            }

#if 1
            // 替换"<" & ">"
            for(auto & ch : content){
                if(ch == '<' || ch == '>'){
                    ch = '\"';
                }
            }
            for(auto & ch : link){
                if(ch == '<' || ch == '>'){
                    ch = '\"';
                }
            }
            for(auto & ch : title){
                if(ch == '<' || ch == '>'){
                    ch = '\"';
                }
            }
#endif

            // 计算simhash
            vector<pair<string ,double>> res; // 关键词序列
            uint64_t u64 = 0; // simhash指纹
            _pSimhasher->extract(content, res, topN);
            _pSimhasher->make(content, topN, u64);
            // 插入vector
            if(hashs.count(u64)){
                // 已存在,去重
                pItemNode = pItemNode->NextSiblingElement("doc");
                continue;
            }
            hashs.insert(u64);
            _pageLib.emplace_back(title,link,content);
            // 获取下一个doc
            pItemNode = pItemNode->NextSiblingElement("doc");
        }
        //delete [] buf;
    }
    ifs.close();
}

void PageLibPreprocessor::buildInvertIndexMap(){
    size_t pageNum = _pageLib.size();
    auto stopWords = _pconf->getStopWords();
    //第一次循环，统计文章内TF & 单词DF & 倒排索引（词频代替权重）
    //记录单词，以及单词出现过的文章篇数。即DF
    map<string,size_t> docCounts;
    //记录所有文章，以及每篇文章的所有单词
    vector<vector<string>> docWords(pageNum);
    for(size_t i = 0 ; i < pageNum; ++i){
        //对一个页面进行分词
        map<string,size_t> wordCounts; // 词频统计
                                       // 分词
        vector<string> temp = _pWordCutter->cutWord(_pageLib[i]._content);
        // 统计
        for(auto & word:temp){
            if(stopWords.count(word)){
                continue; // 停用词跳过
            }
            ++wordCounts[word]; // 词频统计
        }

        //将上面的一个页面内容，汇入总记录
        docWords[i].reserve(wordCounts.size());
        for(auto &word : wordCounts){
            //讲第i篇文章中的单词，汇入总记录下的第i个索引下的vector中
            docWords[i].push_back(word.first);
            //词频统计后，所有单词是不重复的。在当前文章中，该单词如果出现则直接+1,更新DF
            ++docCounts[word.first];
            //键为单词，值为map，单词所在的文章索引作为键，单词在该文章的词频为值(也就是TF,方便后期转换为权重)
            _invertIndexLib[word.first][i] = wordCounts[word.first];
        }
    }

    //第二次循环,计算倒排索引中的权重。 TD-IDF算法
    for(size_t i = 0; i < pageNum; ++i){
        const vector<string>& words = docWords[i];
        for(size_t j = 0; j < words.size(); ++j){
            const string& word = words[j];
            double tf = _invertIndexLib[word][i];
            double df = docCounts[word];
            //计算权重
            _invertIndexLib[word][i] = tf * ::log2(pageNum/(df + 1));
        }
    }

    //第三次循环，对权重进行归一化，防止大文档中的词所占权重过高
    for(size_t i = 0; i < pageNum; ++i){
        const vector<string>& words = docWords[i];
        double totalW = 0;
        for(size_t j = 0; j < words.size(); ++j){
            const string& word = words[j];
            totalW += ::pow(_invertIndexLib[word][i],2);
        }
        totalW = ::sqrt(totalW);
        for(size_t j = 0; j < words.size(); ++j){
            const string& word = words[j];
            _invertIndexLib[word][i] /= totalW;
        }
    }
}

void PageLibPreprocessor::storeOnDisk(){
    // 1 获取配置文件
    auto configs = _pconf->getConfigMap();
    string webpageLib = configs["webpageLib"];
    string offsetLib = configs["offsetLib"];
    string invertIndexLib = configs["invertIndexLib"];
    // 2 网页库 & 网页偏移库
    ofstream ofs1(webpageLib);
    if(!ofs1){
        LogError("open webpageLib failed");
        return;
    }
    ofstream ofs2(offsetLib);
    if(!ofs2){
        LogError("open offsetLib failed");
        return;
    }
    // 2.1 遍历保存
    size_t pageNum = _pageLib.size();
    long totalSize = 0; // pos
    for(size_t i = 0; i < pageNum; ++i){
        ostringstream oss;
        oss << "<doc>\n<title>" << _pageLib[i]._title
            << "</title>\n<link>" << _pageLib[i]._link
            << "</link>\n<content>" << _pageLib[i]._content
            << "</content>\n</doc>\n";
        string resultPage = oss.str();
        ofs1 << resultPage;
        ofs2 << i << " " << totalSize << " " << resultPage.size() << "\n";
        totalSize += resultPage.size();
        /* cout << i << " " << totalSize << " " << resultPage.size() << "\n"; */
    }
    ofs1.close();
    ofs2.close();
    // 3 倒排索引库
    // unordered_map<string,map<size_t,double>> _invertIndexLib;
    ofs1.open(invertIndexLib);
    if(!ofs1){
        LogError("open invertIndexLib failed");
        return;
    }
    /* nlohmann::json root; */
    for(const auto& entry : _invertIndexLib){
        /* nlohmann::json inner_map; */
        ofs1 << entry.first << " ";
        for(const auto& inner_entry : entry.second){
            /* inner_map[std::to_string(inner_entry.first)] = inner_entry.second; */
            ofs1 << inner_entry.first << " " << inner_entry.second << " ";
        }
        /* root[entry.first] = inner_map; */
        ofs1 << "\n";
    }
    ofs1.close();
}

