//echo客户端

#include <iostream> 
#include <include/EventLoop.h>
#include <include/TcpClient.h>
#include <include/IPAddress.h>
#include <include/Buffer.h>
#include <include/TcpConnection.h>

using namespace net;

#ifdef LOG_LEVEL
#undef LOG_LEVEL
#define LOG_LEVEL 0;
#endif

//简单的一个回射服务器
class EchoClient
{
public:
    EchoClient(EventLoop* loop,IPAddress ip)
        :
        _loop(loop),
        _ip(ip),
        _client(loop,_ip)//开启客户端
    {
        _client.setMessageCallback(std::bind(
            &EchoClient::OnRecvDate,this,_1,_2));
        _client.setConnectionCallback(std::bind(
            &EchoClient::OnConnect,this,_1));
    }

    void start()
    {
        _client.start();    //监听
    }


    
private:
    void OnConnect(const TcpConnectionPtr& ptr)
    {
        //开个线程跑发送数据
        TcpConnectionPtr conn = ptr;
        std::thread([this,conn](){
            std::string s;
            while(std::cin>>s)
            {
                conn->send(s.c_str(),s.size());
            }
        });
    }

    void OnRecvDate(const TcpConnectionPtr& ptr,Buffer& buffer)
    {
        char buff[1024];
        int n = buffer.ReadableBytes(); //当前可读字节数
        char a[1024];
        buffer.ReadString(a,n);
        printf("%s\n",a);
    }


private:
    net::EventLoop* _loop;
    net::IPAddress _ip;
    net::TcpClient _client;
};


int main()
{
    net::Logger::GetInstance("./echoclient.log");
    net::EventLoop loop;
    EchoClient client(&loop,IPAddress("127.0.0.1",8888));
    client.start();
    loop.loop();
}