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
#include "Timestamp.h"
#include "Spinlock.h"
#include "TimeTask.h"

namespace net
{


class Timer
{
public:
    Timer();
    ~Timer(){ Stop();};


    bool TryReset(const Timestamp& interval,TimerCallback task);   //尝试set
    bool TryReset(const TimeTask*);   //尝试set
    bool TryReset(int interval,const TimerCallback& task);        //根据时间间隔
private:
    bool StartOnce(const TimeTask*);
    bool StartOnce(const Timestamp& point,const TimerCallback& task);     //根据时间戳
    bool StartOnce(int interval,const TimerCallback& task);        //根据时间间隔
    void Stop();
    bool isInSleep(){ return is_in_sleep;}

private:
    std::atomic<bool> _expired;             //失效标志
    std::atomic<bool> _try_to_expire;       //尝试停止     
    bool is_in_sleep;                       //正在睡眠               
    Spinlock _spin;                         //自旋锁    
    std::atomic<bool> _new_task;            
    int _time;
    TimerCallback _task;                 

};


}

#endif