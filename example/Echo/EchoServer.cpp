#include <iostream> 

#include <include/TcpServer.h>
#include <include/TcpConnection.h>
#include <include/IPAddress.h>
#include <include/Buffer.h>
#include <include/EventLoop.h>

// #include "../include/TcpServer.h"
// #include "../include/IPAddress.h"
// #include "../include/EventLoop.h"
// #include "../include/Buffer.h"
// #include "../include/TcpConnection.h"

using namespace net;

class EchoServer
{
public:
    EchoServer(EventLoop* loop,IPAddress ip)
        :_ip(ip),
        _loop(loop),
        _server(_loop,_ip)
    {
        _server.setMessageCallback(std::bind(
            &EchoServer::OnRecvDate,this,_1,_2));
    }
    
    void start()
    {
        _server.start();
    }

private:
    void OnRecvDate(const TcpConnectionPtr& conn, Buffer& buff  )
    {
        int n = buff.DataSize();
        char a[1024];
        memset(a,'\0',sizeof a);
        buff.ReadString(a,n);
        printf("%s\n",a);
        conn->send(a,n);
    }
private:
    net::IPAddress _ip;
    net::EventLoop* _loop;
    net::TcpServer _server;
};

int main()
{
    net::Logger::GetInstance("./echoserver.log");
    net::EventLoop loop;
    EchoServer server(&loop,IPAddress(8888));
    server.start();
    loop.loop();
}
