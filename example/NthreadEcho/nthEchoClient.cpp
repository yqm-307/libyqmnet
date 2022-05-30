//echo客户端

#include <iostream> 
#include <include/EventLoop.h>
#include <include/TcpClient.h>
#include <include/IPAddress.h>
#include <include/Buffer.h>
#include <include/TcpConnection.h>
// #include "../include/TcpClient.h"
// #include "../include/IPAddress.h"
// #include "../include/EventLoop.h"
// #include "../include/Buffer.h"
// #include "../include/TcpConnection.h"
#include <atomic>
using namespace net;

#ifdef LOG_LEVEL
#undef LOG_LEVEL
#define LOG_LEVEL 0;
#endif

#define MESSAGE_SIZE 100

char* req;

void build()
{
    req = new char[MESSAGE_SIZE];
    memset(req,'1',sizeof req);
}


static std::atomic_int32_t connections(0);
static std::atomic_int64_t transbytes(0);
static std::atomic_int32_t reqs(0);
static std::atomic_int32_t rsps(0); 


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
        connections++;
        //开个线程跑发送数据
        _loop->runEvery(60s,[](){
            printf("总请求数: %d\n",reqs.load());
            printf("总处理请求数: %d\n",rsps.load());
            printf("总流量数: %d byte\n",transbytes.load());
        });
        TcpConnectionPtr conn = ptr;
        conn->send(req,strlen(req));
        reqs++;
    }



    void OnRecvDate(const TcpConnectionPtr& ptr,Buffer& buffer)
    {
        int n = buffer.DataSize(); //当前可读字节数
        char a[128];
        memset(a,'\0',sizeof a);
        buffer.ReadString(a,n);
        transbytes+=n;
        rsps++;
        ptr->send(req,strlen(req));
        reqs++;
    }
private:
    net::EventLoop* _loop;
    net::IPAddress _ip;
    net::TcpClient _client;
};


int main()
{
    build();

    net::Logger::GetInstance("./nthclient.log");
    net::EventLoop loop;
    std::vector<EchoClient*> servs(4,nullptr);
    for(int i=0;i<2;++i)
    {
        servs[i] = new EchoClient(&loop,IPAddress("127.0.0.1",11000));
        //servs[i] = new EchoClient(&loop,IPAddress("192.168.146.129",8888));
        servs[i]->start();
    }
    loop.loop();
    for(auto p : servs)
        delete p;

}