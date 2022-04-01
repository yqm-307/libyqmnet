#include "../include/Timer.h"
#include "../include/Logger.h"
//#include "../include/Buffer.h"
#include "../include/Thread.h"
#include "../include/ThreadPool.h"
#include "../include/EventLoop.h"
#include "../include/TcpConnection.h"
#include "../include/TcpServer.h"

#include <iostream>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>    
#include <chrono>
using namespace net;
using namespace std;

void logtest();
void buffertest();
void timerfdtest();
void testTimer();
void Threadtest();
void ThreadPoolTest();
void testserver();
void testip();
int main()
{
    Timestamp a = clock::now();
    
    //logtest();
    //buffertest();
    //testTimer();
    //Threadtest();
    //ThreadPoolTest();
    testserver();
    //testip();
    Timestamp b = clock::now();
    time_t ms = (b-a).count()/(1000*1000);
    printf("总耗时： %d ms\n",ms);
}


#pragma region IPAddress测试
void testip()
{
    EventLoop loop;
    IPAddress p(8888);
    Acceptor _accept(&loop,p);
    _accept.listen();
    loop.loop();
    // int listenfd = socket(AF_INET,SOCK_STREAM,0);
    // int a=0;
    // if(-1 == setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&a,sizeof(a)))    //重用地址
    //     FATAL("Acceptor::Acceptor() setsockopt:SO_REUSEADDR error!");
    // if(-1 == setsockopt(listenfd,SOL_SOCKET,SO_REUSEPORT,&a,sizeof(a)))    //重用端口
    //     FATAL("Acceptor::Acceptor() setsockopt:SO_REUSEPORT error!");
    
    // if(-1 == ::bind(listenfd,p.getsockaddr(),p.getsocklen()))
    //     {
    //         printf("bind error\n");
    //     }
    // if(-1 == ::listen(listenfd,10))
    // {
    //     printf("listen error!\n");
    // }
    // while(1)
    // {
    //     sockaddr_in addr;
    //     socklen_t len;
    //     int sockfd = accept(listenfd,reinterpret_cast<sockaddr*>(&addr),&len);
    //     char buf[24];
    //     read(sockfd,buf,sizeof(buf));
    //     printf("接受数据:%s\n\n",buf);
    // }
}
#pragma endregion

#pragma region 线程池的使用

void ThreadPoolTest()
{
    Util::ThreadPool<std::function<void()>> pool(200,10000); //创建线程池
    atomic_int dosome(0);
    mutex _lock;

    for(int i=0;i<1000000;++i)
    {
        while(Util::TaskQueueFull == pool.AddTask(
            [&dosome](){
                ++dosome;
                //cout<<"doonce : "<< ++dosome <<endl;
            }));
    }
    sleep(1);
    cout<<"some :"<< dosome<<endl;
    
    //while(1);
}

#pragma endregion

#pragma region 封装线程的测试
void Threadtest()
{
    Util::Thread p[10];
    // std::mutex _mutex;
    // int dosome=0;
    // for(int i=0;i<10;++i)
    // p[i].Start([&dosome,&_mutex]()->Util::ThreadStatus{
    //     {
    //         std::unique_lock<std::mutex> lock(_mutex);
    //                 if(dosome > 100000)
    //         return Util::Stop;
    //         cout<<"线程: "<<gettid()<<"  工作:"<<dosome++<<endl;
    //     }
    //     return Util::Running;
    // });

    p[0].Start([]()->Util::ThreadStatus{

    });
    
    int a;
    while(cin>>a);
    
}

#pragma endregion

#pragma region 测试服务器

class DiscardServer
{
public:
    DiscardServer(EventLoop* loop, const IPAddress& addr)
            : server(loop, addr)
    {
        server.setConnectionCallback(std::bind(
                &DiscardServer::onConnect,this,_1));
        
    }


    void start()
    { server.start(); }


    void onConnect(const TcpConnectionPtr& conn)
    {
        if(conn->connected())
            TRACE("连接建立: %s",conn->name().c_str());
        else
            TRACE("断开连接");
    }

private:
    TcpServer server;
};

void testserver()
{
    //setLogLevel(LOG_LEVEL_TRACE);
    EventLoop loop;
    IPAddress addr(8888);
    DiscardServer server(&loop, addr);
    server.start();
    loop.loop();
}

#pragma endregion

#pragma region 缓冲区测试

// void buffertest()
// {
//     Buffer input;
//     srand(time(NULL));
//     for(int i=0;i++<10;){
//     for(int i=0;i<1000;++i){
//         int a;
//         input.WriteInt32(a=random()%100);
//         TRACE("插入 int32: %d",a);
//         input.Debug();
//     }

//     for(int i=0;i<1000;++i)
//     {
//         int a;
//         a = input.ReadInt32();
//         TRACE("读取 int32 : %d",a);
//         input.Debug();
//     }
//     }


// }

#pragma endregion

#pragma region 日志测试
//日志测试:注意多线程环境，没有模拟业务处理，所以入队操作基本上是瞬间完成。因此
void logtest()
{
    std::cout<<"进入test"<<std::endl;
    //log
    // for(int i=0;i<10000;++i)
    //     INFO("这是第%d条日志\n",i);

    //多线程环境
    for(int i=0;i<100;++i){
        std::thread( [&] (){
            for(int j=0;j<10000;++j)
            INFO("这是日志来自");
        }).join();
    }

}
#pragma endregion

#pragma region 定时器测试

// void work()
// {
//     static int i=0;
//     std::cout<<++i<<std::endl;
// }

void testTimer()
{
    net::Timer timer;    
    time_t now = time(nullptr);
    bool expired=false;
    
    cout<<"启动时间: "<<now<<endl;

    for(int i=0;i<1;++i)
    {
        cout<<1<<endl;
        int changshi=0;
        while(!timer.TryReset(1000,[&expired](){

            time_t now = time(nullptr);
            expired = true;
            printf("苏醒时间 : %d",now);
        }))
        {
            changshi++;
        };
        TRACE("尝试%d次",changshi);
    }
    while(!expired);
}
#pragma endregion