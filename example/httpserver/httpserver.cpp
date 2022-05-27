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
const char* res = 
"HTTP/1.1 200 OK \
Content-Length:219 \
Connection:keep-alive \
\r\n \
<!DOCTYPE html> \
<html> \
    <head> \
        <meta charset=\"UTF-8\"> \
        <title>WebServer</title> \
    </head> \
    <body> \
    hello world \
    </body> \
</html>\r\n";


class httpserver
{
public:
    httpserver(EventLoop* loop,IPAddress ip)
        :_ip(ip),
        _loop(loop),
        _server(_loop,_ip)
    {
        _server.setConnectionCallback(std::bind(
            &httpserver::OnConnection,this,_1));
        _server.setMessageCallback(std::bind(
            &httpserver::OnRecvDate,this,_1,_2));
    }
    
    void start()
    {
        _server.start();
    }

private:
    void OnConnection(const TcpConnectionPtr& conn)
    {
        //conn->send(res,strlen(res));
        //printf("[send]\n%s",res);
    }

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
    net::Logger::GetInstance("./httpserver.log");
    net::EventLoop loop;
    httpserver server(&loop,IPAddress(10100));
    server.start();
    loop.loop();
}
