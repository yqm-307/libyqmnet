#include <iostream>
#include <thread>
#include <atomic>
#include <unistd.h>
#include <include/ThreadPool.h>
#include <include/Timestamp.h>

using namespace std;
using namespace net;
void test();

int main()
{
    Timestamp a = clock::now();
    test();
    Timestamp b = clock::now();
    time_t ms = (b-a).count()/(1000*1000);
    printf("总耗时： %d ms\n",ms-1000);
}
void test()
{
    Util::ThreadPool<std::function<void()>> pool(200,10000); //创建线程池
    atomic_int dosome(0);
    net::Mutex _lock;

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
}