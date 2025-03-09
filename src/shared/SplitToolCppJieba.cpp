#include "../../include/cppjieba/Jieba.hpp"
#include "SplitTool.h"
#include "Configuration.h"

//内部类的实现代码
class SplitToolCppJieba::CppjiebaImpl{
public:
    CppjiebaImpl(Configuration * pconf);
    vector<string> cutWord(const string& sentence);
    ~CppjiebaImpl();
private:
    cppjieba::Jieba *_pjieba;
};

SplitToolCppJieba::CppjiebaImpl::CppjiebaImpl(Configuration *pconf){
    auto configs = pconf->getConfigMap();
    const char* const DICT_PATH = configs["DICT_PATH"].c_str();
    const char* const HMM_PATH = configs["HMM_PATH"].c_str();
    const char* const USER_DICT_PATH = configs["USER_DICT_PATH"].c_str();
    const char* const IDF_PATH = configs["IDF_PATH"].c_str();
    const char* const STOP_WORD_PATH = configs["STOP_WORD_PATH"].c_str();


    _pjieba = new cppjieba::Jieba(DICT_PATH,HMM_PATH,USER_DICT_PATH,IDF_PATH,STOP_WORD_PATH);

}

SplitToolCppJieba::CppjiebaImpl::~CppjiebaImpl(){
    if(_pjieba){
        delete _pjieba;
        _pjieba = nullptr;
    }
}

vector<string> SplitToolCppJieba::CppjiebaImpl::cutWord(const string& sentence){
    vector<string> words;
    _pjieba->Cut(sentence,words);
    return words;
}


//外部类的实现
SplitToolCppJieba::SplitToolCppJieba(Configuration * pconf)
:_pimpl(new CppjiebaImpl(pconf)){}

SplitToolCppJieba::~SplitToolCppJieba(){
    if(_pimpl){
        delete _pimpl;
        _pimpl = nullptr;
    }
}

vector<string> SplitToolCppJieba::cutWord(const string& sentence){
    return _pimpl->cutWord(sentence);
}




