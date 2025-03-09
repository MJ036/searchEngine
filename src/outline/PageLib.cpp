#include <cstddef>
#include <iostream>
#include <sstream>
#include <fstream>
#include "../shared/Configuration.h"
#include "../shared/Mylogger.h"
#include "PageLib.h"
using std::ostringstream;
using std::ofstream;
using std::cout;

// 创建网页库、网页偏移库
void PageLib::create(){
    _dirScanner.traverse(_pconf->getConfigMap()["xml_files"]);
    vector<string> files = _dirScanner.getFiles();

    //total是起始位置，docID是文章ID，pageSize是一个page的大小
    long total = 0;
    size_t docId = 0;
    //遍历所有XML文件，生成网页库、网页偏移库
    for(auto const &file:files){
        vector<string> temp = _fileProcessor.process(file);
        for(auto const &page : temp){
            //添加单个网页
            _pages.emplace_back(page);
            //添加当前网页到偏移库中
            size_t pageSize = page.size();
            _offsetLib[docId] = pair<long,long>{total,pageSize};
            ++docId;
            total += pageSize;
        }
    }
    for(size_t i = 0; i < 10; ++i){ 
        cout << _pages[i]; 
        cout << "id = " << i << "\n" 
            << "pos = " << _offsetLib[i].first << "\n" 
            << "len = " << _offsetLib[i].second << "\n"; 
    } 
    cout << _pages.size() << "\n"; 
    cout << total << "\n"; 
}

void PageLib::store(){
    string webpageLib_old = _pconf->getConfigMap()["webpageLib_old"];
    string offsetLib_old = _pconf->getConfigMap()["offsetLib_old"];

    //1. 存储网页库
    ofstream ofs(webpageLib_old);
    if(!ofs){
        LogError("ofstream open webpageLib_old failed");
        return;
    }

    for(auto const &page : _pages){
        ofs << page;
    }
    ofs.close();

    // 2 存储网页偏移库
    ofs.open(offsetLib_old);
    if(!ofs){
        LogError("ofstream open offsetLib_old failed");
        return;
    }
    for(auto const & offset:_offsetLib){
        ofs << offset.first << " " << offset.second.first << " " << offset.second.second << "\n";
    }
    ofs.close();
}
