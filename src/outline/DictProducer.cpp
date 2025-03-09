#include <cctype>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <istream>
#include <sstream>
#include <iterator>
#include "../shared/Configuration.h"
#include "../shared/SplitTool.h"
#include "DirScanner.h"
#include "DictProducer.h"

using std::istringstream;
using std::ofstream;

DictProducer::DictProducer(const string& dir_eng, SplitTool *pChar)
    :_files(getFiles(dir_eng))
    ,_pcutChar(pChar)
     ,_pcutChinese(nullptr)
{}
DictProducer::DictProducer(const string& dir_zh, SplitTool *pChar,SplitTool *pZh)
    :_files(getFiles(dir_zh))
    ,_pcutChar(pChar)
     ,_pcutChinese(pZh)
{}

vector<string> DictProducer::getFiles(const string& dir){
    DirScanner scanner;
    scanner.traverse(dir);
    return scanner.getFiles();
}

void DictProducer::createIndex(){
    for(size_t i = 0; i < _dict.size(); ++i){
        //把词典中的单词，依次切成字符，返回字符集合
        vector<string> chars = _pcutChar->cutWord(_dict[i].first);
        //字符-包含该字符的单词所在索引i，添加到map中
        for(auto & ch : chars){
            _index[ch].insert(i);
        }
    }
}

void DictProducer::store(){
    string dict_filename, idx_filename;
    if(_pcutChinese == nullptr){
        dict_filename = Configuration::getConfigMap()["dict_eng"];
        idx_filename = Configuration::getConfigMap()["dict_eng_idx"];
    }
    else{
        dict_filename = Configuration::getConfigMap()["dict_zh"];
        idx_filename = Configuration::getConfigMap()["dict_zh_idx"];
    }

    ofstream ofs(dict_filename);
    if(!ofs){
        LogError("open dict_file failed");
        return;
    }
    for(auto & word_pair:_dict){
        ofs << word_pair.first << " " << word_pair.second << "\n";
    }
    ofs.close();
    
    ofs.open(idx_filename);
    if(!ofs){
        LogError("open idx_dile failed");
        return;
    }
    for(auto & char_pair : _index){
        ofs << char_pair.first << " ";
        for(auto idx : char_pair.second){
            ofs << idx << " ";
        }
        ofs << "\n";
    }
}

void DictProducer::buildEnDict(){
    unordered_set<string>& stop_words = Configuration::getStopWords();
    map<string,size_t> wordCounts;
    ifstream ifs;
    for(auto &file : _files){
        ifs.open(file);
        if(!ifs){
            LogError("open dict_file_eng failed");
            return;
        }
        string line;
        while(getline(ifs,line)){
            //先对一整行的单词进行清洗
            for(auto &ch : line){
                if(::ispunct(ch) || ::isdigit(ch)){ch = ' ';}
                if(::isupper(ch)){ ch = ::tolower(ch); }
            }
            istringstream iss(line);
            vector<string> words{std::istream_iterator<string>(iss),std::istream_iterator<string>()};
            for(auto &word : words){
                if(stop_words.count(word)){
                    continue;
                }
                ++wordCounts[word];
            }
        }
        ifs.close();
    }
    for(auto &word_pair : wordCounts){
        _dict.emplace_back(word_pair.first,word_pair.second);
    }
}

void DictProducer::buildCnDict(){
    unordered_set<string>& stop_words = Configuration::getStopWords();
    map<string,size_t> wordCounts;
    ifstream ifs;
    for(auto &file : _files){
        ifs.open(file);
        if(!ifs){
            LogError("open dict_files_zh failed");
            return;
        }
        string line;
        while(getline(ifs,line)){
            //中文分词
            vector<string> words = _pcutChinese->cutWord(line);
            //过滤ASCII(英文、数字)、停用词。并统计词频
            for(auto & word : words){
                if(((word[0] & 0x80) == 0) || stop_words.count(word)){
                    continue;
                }
                ++wordCounts[word];
            }
        }
        ifs.close();
    }
    for(auto& word_pair:wordCounts){
        _dict.emplace_back(word_pair.first,word_pair.second);
    }
}
