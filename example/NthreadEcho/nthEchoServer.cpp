
#include <include/EventLoop.h>
#include <include/TcpServer.h>
#include <include/IPAddress.h>
#include <include/Buffer.h>
#include <include/TcpConnection.h>
// #include "../include/TcpServer.h"
// #include "../include/IPAddress.h"
// #include "../include/EventLoop.h"
// #include "../include/Buffer.h"
// #include "../include/TcpConnection.h"
#include <utility>
#include <vector>
#include <stdio.h>
#include <unistd.h>

using namespace net;

int numThreads = 0;

class EchoServer
{
public:
    EchoServer(EventLoop* loop,IPAddress& ip)
        :server_(loop,ip),
        loop_(loop),
        time_(false)
    {
        server_.setMessageCallback(std::bind(&EchoServer::OnMessage,this,_1,_2));
        server_.setNumThread(2);
    }
    ~EchoServer()
    {
        for(auto p : conns_)
        {
            p->shutdown();
        }
    }

    void start()
    {
        server_.start();
    }
private:
    
    void OnMessage(const TcpConnectionPtr& conn, Buffer& buff)
    {
        int n = buff.DataSize();
        char a[128];
        memset(a,'\0',sizeof a);
        buff.ReadString(a,n);
        conn->send(a,n);
    }
private:
    TcpServer server_;
    EventLoop* loop_;
    std::atomic_bool time_;
    std::vector<TcpConnectionPtr> conns_;
    Timestamp startTime_;
};

class ServerManager
{
public:
    ServerManager(IPAddress& addr,int nthreads = 8)
        :nthreads_(nthreads)
    {
        for(int i=0;i<nthreads_;++i)
        {
            
            //servers_.push_back(new EchoServer());
        }
    }

private:
    std::vector<EchoServer*> servers_;
    std::vector<EventLoop*> loops_;
    int nthreads_;
};


int main(int argc, char* argv[])
{
    net::Logger::GetInstance("nthserver.log");
    EventLoop loop;
    IPAddress listenAddr(11000);
    EchoServer server(&loop, listenAddr);
    server.start();

    loop.loop();
}