#include <iostream>
#include <include/Thread.h>
#include <include/ThreadPool.h>
#include <include/TcpConnection.h>  
#include <include/IPAddress.h>
#include <include/TcpServer.h>
#include "codec.h"
#include "message.h"


class nthserver
{
public:
    typedef std::vector<net::EventLoop*> EventLoopList;

    nthserver(net::EventLoop* loop,net::IPAddress&& addr,size_t nthread)    
        :_ioloop(loop),
        _thread_num(nthread),
        _server(_ioloop,addr)
    {
        using namespace net;
        
        _server.setConnectionCallback(std::bind(&nthserver::OnConnection,this,_1));
        _server.setMessageCallback(std::bind(&nthserver::OnRecvData,this,_1,_2));
    }

    void OnConnection(const net::TcpConnectionPtr& conn)
    {
        INFO("connection %s -> %s %s ",
        conn->peer().GetIPPort().c_str(),
        conn->local().GetIPPort().c_str(),
        conn->isconnected() ? "up":"down");
    }
    void OnRecvData(const net::TcpConnectionPtr& conn,net::Buffer& buff)
    {

    }

private:
    net::EventLoop* _ioloop;    //io线程，主线程
    size_t _thread_num;
    EventLoopList _loops;       //其他线程中的其他循环
    
    Message _msg;
    Util::Thread _thread;
    net::TcpServer _server;
};

int main()
{
    
}