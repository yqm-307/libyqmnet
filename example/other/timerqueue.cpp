#include <include/TimerQueue.h>
#include <include/EventLoop.h>

using namespace net;


void test1(net::EventLoop& loop)
{
    net::timetask_t timeid = loop.runEvery(3s,[](){
        Timestamp n = net::clock::now();
        
        printf("%d年%d月%d日  %d:%d:%d  %dms\n"
        ,net::clock::year()
        ,net::clock::month()+1
        ,net::clock::day()
        ,net::clock::hour()
        ,net::clock::minute()
        ,net::clock::second()
        ,net::clock::millisecond());
    });
    net::timetask_t ti = loop.runAfter(1s,[](){
        printf("1s after\n");
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