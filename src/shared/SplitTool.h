#ifndef __SPLITTOOLH__
#define __SPLITTOOLH__

#include<string>
#include<vector>


using std::vector;
using std::string;

class SplitTool
{
public:
    virtual vector<string> cutWord(const string& sentence) = 0;
    virtual ~SplitTool(){};

};

//前向声明
class Configuration;
class SplitToolCppJieba
:public SplitTool
{
    public:
        SplitToolCppJieba(Configuration * pconf);
        ~SplitToolCppJieba();
        vector<string> cutWord(const string& sentence) override;
    private:
        class CppjiebaImpl;
        CppjiebaImpl * _pimpl;
};

//用于构造索引文件
class SplitToolChar
:public SplitTool
{
public:
    //切割单个单词，返回 构成单词的字符集合
    vector<string> cutWord(const string& sentence) override;
};

#endif //__SPLITTOOLH__


