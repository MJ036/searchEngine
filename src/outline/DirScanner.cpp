#include <dirent.h>

#include "../shared/Mylogger.h"
#include "DirScanner.h"

void DirScanner::traverse(const string& dir){
    //保存目录流的指针
    DIR *directory;
    //保存目录信息的实体
    struct dirent *ent;
    //打开目录流
    if((directory = ::opendir(dir.c_str())) != nullptr){
        //获取目录文件的信息
        while((ent = ::readdir(directory)) != nullptr){
            //将常规文件保存在_file中
            if(ent->d_type == DT_REG){
                _files.push_back(dir + "/" + ent->d_name);
            }
        }
        ::closedir(directory);
    }else{
        LogError("Error opening directory:" + dir);
    }
}
