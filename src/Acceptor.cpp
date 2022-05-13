#include <sys/socket.h>
#include <netinet/in.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>

#include "../include/Acceptor.h"
#include "../include/Logger.h"
#include "../include/EventLoop.h"
using namespace net;

void setfdnonblock(int fd)
{
    int old = fcntl(fd,F_GETFL);
    int newopt = old | O_NONBLOCK | O_CLOEXEC;
    if(-1 == fcntl(fd,F_SETFL,newopt))
        FATAL("Acceptor::setfdnonblock() fcntl error!");
    return;
}
int createsockfd()
{
    int openopt = 1;
    int ret = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC , 0);
    if (ret == -1)
        FATAL("Acceptor::createsockfd create sockfd error!");
    int setopt = 0;
    setopt = setsockopt(AF_INET,SOL_SOCKET,SO_REUSEPORT,&openopt,sizeof(openopt));
    if (setopt == -1)
        FATAL("Acceptor::createsockfd setsockopt error");
    setopt = setsockopt(AF_INET,SOL_SOCKET,SO_REUSEADDR,&openopt,sizeof(openopt));
    if (setopt == -1)
        FATAL("Acceptor::createsockfd setsockopt error");
    return ret;
}


Acceptor::Acceptor(EventLoop* loop,const IPAddress& ipport)
        :
        _listening(false),
        _loop(loop),
        _listenfd(createsockfd()),
        _channel(_loop,_listenfd),
        _ip(ipport)
{
    assert(_listenfd != -1);
    if(-1 == bind(_listenfd,_ip.getsockaddr(),_ip.getsocklen()))
        FATAL("Acceptor::Acceptor() bind error!");
}


Acceptor::~Acceptor()
{
    close(_listenfd);
}


void Acceptor::SetOnConnect(OnConnectCallback cb)
{
    //function空 异常报错
    if(cb == nullptr)
        FATAL("Accept::SetOnConnect error!  cb is nullptr");
    _onconnectcb = cb;
}

//acceptor是否正在listen
bool Acceptor::listening()
{
    return _listening;
}

//开始监听
void Acceptor::listen()
{
        //监听套接字
    _loop->assertInLoopThread();
    if(-1==::listen(_listenfd,1024))
        FATAL("Acceptor::listen() error!");
    _listening = true;
    _loop->isInLoopThread();
    _channel.setReadCallback([this](){handleRead();});
    _channel.enableRead();
}


//accept
void Acceptor::handleRead()
{
    _loop->assertInLoopThread();

    struct sockaddr_in _addr;
    void* o = &_addr;
    socklen_t len;
    int _clifd;
    if(-1 == (_clifd = ::accept(_listenfd,reinterpret_cast<sockaddr*>(o),&len)))
    {
        FATAL("Acceptor::handleRead() accept() error!");
    }
    setfdnonblock(_clifd);  //客户端连接

    if (_onconnectcb) { //有回调，call
        IPAddress peer;
        peer.set(_addr);
        _onconnectcb(_clifd, _ip, peer);
    }
    else    //没有回调直接关闭
    {
        close(_clifd);
    }

}