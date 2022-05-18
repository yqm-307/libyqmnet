#ifndef TIMER_H
#define TIMER_H

/*
    定时器使用在计时器队列中，在Timequeue中的话一次只触发一次，所以先做一个一次执行一次cb的接口
    
    不需要特化，只针对网络库中的使用做一个一次callback一次的接口

*/
#include <thread>
#include <time.h>
#include <chrono>
#include <functional>
#include <atomic>
#include <memory>
#include <condition_variable>           //信号量
#include <iostream>
#include <chrono>
#include "CallBack.h"
#include "ThreadPool.h"
#include "Timestamp.h"
#include "Spinlock.h"
#include "TimeTask.h"

namespace net
{

class Timer:std::enable_shared_from_this<Timer>
{
public:
    Timer(Millisecond tick,TimerCallback cb);
    Timer()=default;
    ~Timer(){Stop();}
    void Start();
private:
    void Stop();

private:
    std::atomic_bool    _running;
    Millisecond         _time;
    TickCallbcak _task;     //tick                 
};


}

#endif