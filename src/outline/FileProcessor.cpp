#include <openssl/crypto.h>
#include <regex>
#include <sstream>
#include "../shared/tinyxml2.h"
#include "../shared/Mylogger.h"
#include "FileProcessor.h"
using namespace tinyxml2;
using std::ostringstream;

struct RSSItem{
    RSSItem(const string& title, const string& link, const string& description)
        :_title(title),_link(link),_description(description)
    {}

    string _title;
    string _link;
    string _description;
};

vector<string> FileProcessor::process(const string& fileName){
    vector<RSSItem> _items;
    _items.reserve(20);

    XMLDocument doc;
    //加载XML文件
    doc.LoadFile(fileName.c_str());
    if(doc.ErrorID()){
        LogError("XML loadfile failed\n");
        return vector<string>();
    }

    //逐级获取到指向item内容的指针
    XMLElement *pItemNode = doc.FirstChildElement("rss")
        ->FirstChildElement("channel")
        ->FirstChildElement("item");
    // 循环处理所有的item
    while(pItemNode){
        string title,link,description;
        //先处理description
        XMLElement *temp = pItemNode->FirstChildElement("description");
        if(temp == nullptr){
            pItemNode = pItemNode->NextSiblingElement("item");
            continue;
        }
        description = temp->GetText();
        //清理description中的正则表达式
        std::regex reg("<[^>]*>");
        description = std::regex_replace(description,reg,"");
        //如果description为空，直接处理下一个item
        if(description.size() == 0){
            pItemNode = pItemNode->NextSiblingElement("item");
            continue;
        }

        temp = pItemNode->FirstChildElement("title");
        if(temp != nullptr){
            title = temp->GetText();
            title = std::regex_replace(title,reg,"");
        }
        temp = pItemNode->FirstChildElement("link");
        if(temp != nullptr){
            link = temp->GetText();
        }
        // 插入vector
        _items.emplace_back(title,link,description);
        // 获取下一个item结点
        pItemNode = pItemNode->NextSiblingElement("item");
    }//遍历完所有的文章

    //用vector<string>存储，处理过的网页内容,并返回
    vector<string> result;
    result.reserve(_items.size());
    for(size_t idx = 0; idx != _items.size(); ++idx){
        ostringstream oss;
        oss << "<doc>\n<title>" << _items[idx]._title
            << "</title>\n<link>" << _items[idx]._link
            << "</link>\n<content>" << _items[idx]._description
            << "</content>\n</doc>\n";
        result.push_back(oss.str());
    }
    return result;
}
