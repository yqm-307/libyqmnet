#include <libyqmnet/TcpConnection.h>
#include <libyqmnet/EventLoop.h>
#include <libyqmnet/TcpServer.h>
#include <libyqmnet/IPAddress.h>
#include <libyqmnet/Logger.h>
#include <functional>
#include <thread>
#include <iostream>

using namespace std;
using namespace net;
const char* res = "HTTP/1.1 200 OK\r\n\
Content-Length:98\r\n\
Connection:keep-alive\r\n\
<html>\
<h>hello world</h>\
</html>\r\n";


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
         conn->send(res,strlen(res));
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
