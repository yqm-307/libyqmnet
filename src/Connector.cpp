
#include <unistd.h>
#include <sys/socket.h>
#include <cassert>

#include "../include/IPAddress.h"
#include "../include/EventLoop.h"
#include "../include/Logger.h"
#include "../include/Connector.h"

using namespace net;

Connector::Connector(EventLoop* loop,IPAddress ip)
        :_loop(loop),
        _addr(ip),
        _started(false),
        _connected(false),
        _sockfd(::socket(AF_INET,SOCK_STREAM | SOCK_CLOEXEC,0)),
        _channel(_loop,_sockfd)
{
    if(-1==_sockfd)
        FATAL("Connector::Connector() creaete socket error!");
    _channel.setWriteCallback([this](){handleWrite();});
    _channel.enableWrite();
}

Connector::~Connector()
{
    ::close(_sockfd);
}


//建立_addr的连接
void Connector::connect()
{
    _loop->assertInLoopThread();
    assert(!_started);
    _started = true;

    int servfd;
    if( -1 == (servfd = ::connect(_sockfd,_addr.getsockaddr(),_addr.getsocklen())))
        FATAL("Connector::connect() ::connect error!");
    else
        handleWrite();
}

void Connector::handleWrite()
{
    _loop->assertInLoopThread();
    assert(_started);
    _loop->removeChannel(&_channel);    //删除当前


    int err;    //错误
    socklen_t len = sizeof(err);
    if(-1 != ::getsockopt(_sockfd, SOL_SOCKET, SO_ERROR, &err, &len));  //获取错误码
        errno = err;

    //socket错误，error回调
    if (errno != 0) {
        FATAL("Connector::handleWrite socket error!");
        if (_errorcb)
            _errorcb();
    }
    else if (_newconnectcb) {  //没有错误，OnConnectCallback
        struct sockaddr_in addr;
        len = sizeof(addr);
        void* o = &addr;
        int ret = ::getsockname(_sockfd, static_cast<sockaddr*>(o), &len);
        if (ret == -1)
            ERROR("Connection::getsockname()");
        IPAddress local;
        local.set(addr);

        //回调吧sockfd控制权转移，但是生命期还是在当地
        _connected = true;  //连接流程完成
        _newconnectcb(_sockfd, local, _addr);   //连接回调传出，移交控制权
    }
}