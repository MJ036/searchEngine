#ifndef __CONFIGURATION_H__
#define __CONFIGURATION_H__

#include<string>
#include<map>
#include<unordered_set>
#include <fstream>
#include "Mylogger.h"
using std::string;
using std::map;
using std::unordered_set;
using std::ifstream;

class Configuration
{
public:

private:
    Configuration();
public:
    //配置文件的目录在静态区，本项目直接在初始化时指定了
    //该函数目前没什么用
    static void setConFilePath(const string& filename){ _confFilePath = filename; }

    static Configuration * getInstance(){
        if(!_pInstance){
            //单例模式自动释放,以免valgrind报错
            ::atexit(destroy);
            _pInstance = new Configuration();
        }
        return _pInstance;
    }
    static void destroy(){
        if(_pInstance){
            delete _pInstance;
            _pInstance = nullptr;
        }
    }
    static map<string,string>& getConfigMap(){ return getInstance()->_configs; }
    static unordered_set<string>& getStopWords(){ return getInstance()->_stopWord; }
private:
    static Configuration* _pInstance;
    static string _confFilePath;
    map<string,string> _configs;
    unordered_set<string> _stopWord;

};

#endif


