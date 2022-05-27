#ifndef CALLBACK_H
#define CALLBACK_H

#include <functional>
#include <memory>
#include "Config.h"
#include "IPAddress.h"

namespace net
{

class TcpConnection;
class Buffer;

//函数占位符
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;
using std::placeholders::_5;

//TcpConnection
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef std::function<void(const TcpConnectionPtr&)> CloseCallback;
typedef std::function<void(const TcpConnectionPtr&)> ConnectionCallback;                //连接建立和断开时回调
typedef std::function<void(const TcpConnectionPtr&)> WriteCompleteCallback;             
typedef std::function<void(const TcpConnectionPtr&, size_t)> HighWaterMarkCallback;
typedef std::function<void(const TcpConnectionPtr&, Buffer&)> MessageCallback;
typedef std::function<void(const TcpConnectionPtr&, Buffer&)> DeCodeFunc;
typedef std::function<void(const char*,size_t)> EnCodeFunc;



typedef std::function<void()> TickCallbcak;     //Timer 滴答callback
typedef std::function<void()> TimerCallback;    //计时器的callback
typedef std::function<void(size_t index)> ThreadInitCallback;   //线程初始化回调函数
typedef std::function<void(int sockfd,const IPAddress&, const IPAddress&)> OnConnectCallback;   //新连接的sockfd,本地地址,新连接地址
typedef std::function<void()> ErrorCallback;
typedef std::function<void()> Task;


void defaultConnectionCallback(const TcpConnectionPtr& conn);
void defaultMessageCallback(const TcpConnectionPtr& conn,Buffer& buff);
void defaultThreadInitCallback(size_t index);


typedef uintptr_t timetask_t; 
}



#endif 