#ifndef BUFFER_H
#define BUFFER_H
/*
    buffer是在用户调用TcpConnection 的 send 可以不阻塞的方法；send之后写入buffer，就立即返回，不会阻塞在系统调用的write中

    不是线程安全的，但是一个buffer只在一个connection中，只有一个收发线程。不会出现临界区
*/
#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <cassert>
#include <cstring>
#include "Config.h"
#include "Logger.h"

namespace net
{
class Buffer
{
public:
    static const int headSize;  //可以插入消息id或者长度
    static const int initSize;
    Buffer(size_t initSize = initSize);
    ~Buffer(){}

    //常见值的读写
    void swap(Buffer& buffer);          //拷贝一个buffer
    void InitAll();                 //初始化
    bool WriteInt64(int64_t num);
    bool WriteInt32(int32_t num);
    bool WriteInt16(int16_t num);
    bool WriteInt8(int8_t num);
    bool WriteString(std::string str);
    bool WriteString(const char* p , int len);
    int64_t ReadInt64();
    int32_t ReadInt32();
    int16_t ReadInt16();
    int8_t  ReadInt8();
    void ReadString(std::string& ,int len);
    void ReadString(char*,int len);

    //当前peek
    char* peek();
    void recycle(size_t n);                 //回收n字节空间
    size_t ReadableBytes();                 //剩余可读字节
    size_t WriteableBytes();                //剩余可写字节
    size_t PrepareBytes();                  //前置空间
    size_t DataSize(){return ReadableBytes();}  //buffer 数据段长度

    int64_t readfd(int sockfd,int& Errno);  //connector接受数据使用

private:   
    bool Read(void* ,int len);                          //最终实现Read
    bool Write(const char* data, size_t len);           //最终实现write
    
    char* begin()                               
    {return &*bytes.begin();}
    const char* begin() const                   
    {return &*bytes.begin();}
    void move(int start,int len,int obj);       //移动
    void moveForward();                         //向前移动
private:
    std::vector<char> bytes;            //比特流
    size_t _readIndex;                  //已读
    size_t _writeIndex;                 //已写
    const int reservedBytes;            //预留位置

    const char CRLF[3] = "\r\n";
};


}

#endif