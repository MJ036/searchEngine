#ifndef _DICTPRODUCER_H
#define _DICTPRODUCER_H

#include <vector>
#include <string>
#include <set>
#include <unordered_map>
using std::vector;
using std::string;
using std::set;
using std::pair;
using std::unordered_map;

class SplitTool;
class DictProducer
{
public:
    //处理英文
    DictProducer(const string& dir_eng, SplitTool *pChar);
    //处理中文
    DictProducer(const string& dir_zh, SplitTool *pChar,SplitTool *pZh);
    //构建词典
    void buildDict();
    //构建词典索引
    void createIndex();
    //持久化存储
    void store();
private:
    void buildEnDict();
    void buildCnDict();
    vector<string> getFiles(const string& dir);
private:
    //从目录中提取出的素材文件集合
    vector<string> _files;
    //词典
    vector<pair<string,size_t>> _dict;
    //字母 - 包含该字母的单词 所在行 的集合
    unordered_map<string,set<size_t>> _index;
    //指向分词工具类的指针
    SplitTool * _pcutChar;
    SplitTool * _pcutChinese;
};

inline void DictProducer::buildDict(){
    if(_pcutChinese == nullptr){
        buildEnDict();
    }else{
        buildCnDict();
    }
}
#endif


