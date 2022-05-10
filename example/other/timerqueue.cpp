#include <include/TimerQueue.h>
#include <include/EventLoop.h>

using namespace net;


void test1(net::EventLoop& loop)
{
    net::timetask_t timeid = loop.runEvery(3s,[](){
        Timestamp n = net::clock::now();
        printf("time now : %ld\n",n.time_since_epoch().count()/1000/1000);
    });
    getchar();
    loop.cancelTimer(timeid);
}

int main()
{
    net::EventLoop loop;
    test1(loop);

    loop.loop();
}