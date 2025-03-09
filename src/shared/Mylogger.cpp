#include "Mylogger.h"
#include <log4cpp/Appender.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/RollingFileAppender.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/Priority.hh>

using namespace log4cpp;

#define LOGSIZE (10*1024)
#define RFLOGNUM 3
#define LOGFORM "%d %c [%p] %m%n"

Mylogger* Mylogger::_pInstance = nullptr;

Mylogger* Mylogger::getInstance(){
    if(!_pInstance){
        _pInstance = new Mylogger();
    }
    return _pInstance;
}

void Mylogger::destroy(){
    if(_pInstance){
        delete _pInstance;
        _pInstance = nullptr;
    }
}

Mylogger::Mylogger()
//设置记录器
:_mycat(Category::getInstance("mycat"))
{
    ::atexit(&Mylogger::destroy);
    //设置过滤器
    _mycat.setPriority(Priority::DEBUG);
    //设置输出器
    RollingFileAppender *rfAppender = new RollingFileAppender("rfApp","../log/rflog",LOGSIZE,RFLOGNUM);
    //设置格式化器
    PatternLayout *rfPtn = new PatternLayout();
    rfPtn->setConversionPattern(LOGFORM);
    rfAppender->setLayout(rfPtn);
    //为记录器绑定输出器
    _mycat.addAppender(rfAppender);
}


Mylogger::~Mylogger(){
    Category::shutdown();
}

void Mylogger::error(const char *msg){
    _mycat.error(msg);
}
void Mylogger::warn(const char *msg){
    _mycat.warn(msg);
}
void Mylogger::info(const char *msg){
    _mycat.info(msg);
}
void Mylogger::debug(const char *msg){
    _mycat.debug(msg);
}



