#include "Configuration.h"
#include "Mylogger.h"

//在实现文件中首先初始化静态成员
string Configuration::_confFilePath = "../conf/myconf.conf";
Configuration *Configuration::_pInstance = nullptr;

Configuration::Configuration(){
    
    //读取配置文件
    ifstream ifs(_confFilePath);
    if(!ifs){
        LogError("open conf_file failed\n");
        return;
    }
    string key,value;
    while(ifs){
        ifs >> key >> value;
        _configs[key] = value;
    }
    ifs.close();
    //添加中文停用词
    ifs.open(_configs["stop_words_zh"]);
    if(!ifs){
        LogError("open stop_words_zh failed\n");
        return;
    }
    while(ifs){
        ifs >> key;
        _stopWord.emplace(key);
    }
    ifs.close();
    //添加英语停用词
    ifs.open(_configs["stop_words_eng"]);
    if(!ifs){
        LogError("open stop_words_eng failed\n");
        return;
    }
    while(ifs){
        ifs >> key;
        _stopWord.emplace(key);
    }
    ifs.close();

    //在停用词中添加 空格 和 换行符
    _stopWord.emplace(" ");
    _stopWord.emplace("\n");
    

}

