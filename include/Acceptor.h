#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include "Config.h"
#include "noncopyable.h"
#include "IPAddress.h"
#include "Channel.h"
#include "CallBack.h"

/*
    给服务端用的：
    功能： 监听端口，有新连接时回调
*/

namespace net
{
class EventLoop;

class Acceptor:noncopyable
{
public:
    Acceptor(EventLoop*,const IPAddress&);
    ~Acceptor();
    void listen();
    bool listening();

    
    void SetOnConnect(OnConnectCallback cb);   //新连接到达
private:
    void handleRead();  //主要函数



    bool _listening;    //是否正在监听
    EventLoop* _loop;   
    int _listenfd;        //套接字
    Channel _channel;  //
    IPAddress _ip;
    OnConnectCallback _onconnectcb;     //新连接到达cb
};

}



#endif
