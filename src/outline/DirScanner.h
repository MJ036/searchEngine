#ifndef _DIRSCANNER_H
#define _DIRSCANNER_H

#include <string>
#include <vector>
using std::string;
using std::vector;

class DirScanner
{
public:
    //扫描目录，将dir目录中的regular file 保存在_file中
    void traverse(const string& dir);
    //获取被扫描目录下的文件
    vector<string>& getFiles(){return _files;};
private:
    vector<string>_files;

};
#endif


