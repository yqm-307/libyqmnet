#include "../include/Timer.h"
#include <iostream>
#include <stdio.h>
#include <assert.h>

using namespace net;
using namespace clock;

Timer::Timer()
    :_expired(true),
    _try_to_expire(false),
    is_in_sleep(false),
    _new_task(false),
    _task()
{
    //开启一个线程
    std::thread([this](){
        while(!_try_to_expire){ //一直运行，除非主动关停            
            //当前任务
            if(!_expired)
            {
                //新的任务
                TimerCallback _callback = _task;
                int ms = _time;
                //可以改变cb和time了                
                printf("休眠%dms",ms);
                is_in_sleep = true; //tryreset可以返回了
                std::this_thread::sleep_for(std::chrono::milliseconds(ms));   //休眠指定毫秒数
                _callback();

                is_in_sleep = false;    //苏醒了
                _expired = true;        //计时器超时过了
            }
    }
    _try_to_expire = false;
    }).detach();
}


bool Timer::StartOnce(const TimeTask* tsk)
{
    return StartOnce(tsk->When(),tsk->GetCallback());
}

//根据时间点定时器
bool Timer::StartOnce(const Timestamp& time ,const TimerCallback& task)
{
    time_t ms = 0;
    assert(time>now());

    Nanosecond tmp = time - now();      //ns
    if(tmp < 1ms)
        ms = 1;                         //至少1ms
    else
        ms = tmp.count() /(1000*1000);

    return StartOnce(ms,task);
}

//时间间隔定时器
bool Timer::StartOnce(int time,const TimerCallback& task)
{
    if(!_expired)
        return false;
    _time = time;
    _task = task;
    _expired = false;   //线程可以结束轮询，进入sleep中了
    while(is_in_sleep);    //等到线程开始睡眠就退出，防止睡眠前再次改变cb
    return true;
}

void Timer::Stop()
{
    //不重复停止
    if(_try_to_expire)
    {
        return;
    }
    _try_to_expire = true;
}

bool Timer::TryReset(const Timestamp& tpoint, TimerCallback task)
{
    if(is_in_sleep) //正在休眠，失败
    {
        return false;       //reset失败
    }
    return StartOnce(tpoint,task);
}

//内部程序使用
bool Timer::TryReset(const TimeTask* tsk)
{
    if(tsk->isCanceled())
        return true;
    if(is_in_sleep)   //当前事件没有失效
    {
        return false;
    }
    //已经失效了，可以执行新任务
    return StartOnce(tsk);
}

bool Timer::TryReset(int interval,const TimerCallback&task)        //根据时间间隔
{
    if(is_in_sleep)
    {
        return false;
    }
    return StartOnce(interval,task);
}




