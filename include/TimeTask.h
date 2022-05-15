#ifndef TIMETASK_H
#define TIMETASK_H

/*
    一个计时事件
    把回调和时间等设置封装起来
*/
#include "CallBack.h"
#include "Timestamp.h"
#include <chrono>

namespace net
{

class TimeTask{

public:
    // 超时时间+超时回调+是否自动重启
    TimeTask(Timestamp ts,TimerCallback cb,Millisecond interval=0ms)
        :_callback(cb),
        _stamp(ts),
        _interval(interval),
        _is_canceled(false)
    {
    }

    TimeTask()
    :_callback(NULL),
    _stamp(clock::now()),
    _interval(0ms),
    _is_canceled(false)
    {
    }


    bool operator==(TimeTask& obj)
    {   return this->_interval == obj.interval();}
    bool operator!=(TimeTask& obj)
    {   return this->_interval != obj.interval();}
    bool operator>(TimeTask& obj)
    {   return this->_interval>obj.interval();}
    bool operator<(TimeTask& obj)
    {   return !(*this>obj)&&*this!=obj;}

    bool isAutoReset()  {return _interval.count() > 0;}
    bool isCanceled() const   {return _is_canceled;}    
    void handle()   {_callback();}
    void cancel()   {_is_canceled = true;}  //取消后，定时器执行不休眠，直接返回
    Millisecond interval()  {return _interval;}
    Timestamp When() const  {return _stamp;} 
    TimerCallback GetCallback() const {return _callback;}
    //时间间隔更新新的超时时间
    void updateStamp()
    {
        _stamp += _interval;
    }

private:
    Millisecond     _interval;      //时间间隔  ms
    TimerCallback   _callback;      //超时回调
    Timestamp       _stamp;         //超时时间  ns
    std::atomic_bool            _is_canceled;   //是否被取消了
};



}


#endif