#include <iostream>
#include <thread>
#include <atomic>
#include <unistd.h>
#include <include/ThreadPool.h>
#include <include/Timestamp.h>
#include <include/MyLocker.h>
#include <include/Logger.h>
// #include "../include/ThreadPool.h"
// #include "../include/Timestamp.h"
// #include "../include/MyLocker.h"
// #include "../include/Logger.h"


using namespace std;
using namespace net;
typedef Util::ThreadPool<std::function<void()>> tPool;

void test(tPool&);
void test2(tPool&);


int main()
{
    tPool pool(8); //创建线程池

    Timestamp a = clock::now();
    test(pool);
    Timestamp b = clock::now();
    time_t ms = (b-a).count()/(1000*1000);
    printf("CountDownLatch 总耗时： %d ms\n",ms);


    a=clock::now();
    test2(pool);
    b=clock::now();
    printf("atomic + cond 总耗时：%d ms\n",(b-a).count()/(1000*1000));
}
void test(tPool& pool)
{

    net::CountDownLatch latch(1000000);
    //主线程
    std::thread t([&pool,&latch]()
    {
        for(int i=0;i<1000000;++i)
        {
            while(Util::ThreadPoolErrnoCode::TaskQueueFull==pool.AddTask(
                [&latch](){
                    latch.down();
                }));
        }
    });
    t.detach();

    latch.wait();   //计数为0前阻塞
}


void test2(tPool& pool)
{

    net::Sem_t sem;
    std::atomic_int t(0);
    //主线程
    std::thread th([&pool,&t,&sem]()
    {
        for(int i=0;i<1000001;++i)
        {
            while(Util::ThreadPoolErrnoCode::TaskQueueFull==pool.AddTask(
                [&t,&sem](){
                    if((t++) >= 1000000)
                        sem.notify_all();
                }));
        }
    });
    th.detach();
    sem.wait();
    INFO("sem = %d",t.load());
}

/*
性能差距不大，可以看出atomic本质上也是加锁。且两种方式使用信号量同步效果几乎一致。

CountDownLatch 总耗时： 916 ms
atomic + cond 总耗时：878 ms

CountDownLatch 总耗时： 861 ms
atomic + cond 总耗时：820 ms

CountDownLatch 总耗时： 852 ms
atomic + cond 总耗时：775 ms

CountDownLatch 总耗时： 844 ms
atomic + cond 总耗时：840 ms

CountDownLatch 总耗时： 917 ms
atomic + cond 总耗时：826 ms

CountDownLatch 总耗时： 871 ms
atomic + cond 总耗时：857 ms

CountDownLatch 总耗时： 934 ms
atomic + cond 总耗时：876 ms

CountDownLatch 总耗时： 879 ms
atomic + cond 总耗时：845 ms


*/