#include <iostream> 
#include <unistd.h>
#include <include/Thread.h>
#include <include/ThreadPool.h>

using namespace std;
using namespace net;
void test();

int main()
{
    Timestamp a = clock::now();
    
    test();
    
    Timestamp b = clock::now();
    time_t ms = (b-a).count()/(1000*1000);
    printf("总耗时： %ld ms\n",ms);
}



void test()
{
    //开启10个线程
    Util::Thread p[10];
    std::mutex _mutex;
    int dosome=0;
    for(int i=0;i<10;++i)
    p[i].Start([&dosome,&_mutex]()->Util::ThreadStatus{
        {
            std::unique_lock<std::mutex> lock(_mutex);
            if(dosome > 100000)
                return Util::Stop;
            std::cout<<"线程: "<<::gettid()<<"  工作:"<<dosome++<<std::endl;
        }
        return Util::Running;
    });

    // p[0].Start([]()->Util::ThreadStatus{
    // });
    
    int a;
    while(cin>>a);   
}