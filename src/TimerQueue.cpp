#include <string>
#include <algorithm>
#include <thread>
#include "../include/TimerQueue.h"

using namespace net;

#define Task(xx) (*this)

TimerQueue::TimerQueue(EventLoop* loop)
    :_loop(loop),
    _timer(10ms,[this](){handle();}),
    _lock(),
    _async_evt_pool(10),
    _timetasks([](timetask_t a,timetask_t b)->bool{
                return reinterpret_cast<TimeTask*>(a)->When() < reinterpret_cast<TimeTask*>(b)->When();
            })
{
}



TimerQueue::~TimerQueue()
{
    TimeTask* ptr=nullptr;
    for(auto& p:_timetasks)
    {
        ptr = p.second;
        delete ptr;
    }
}

//对于autoreset事件，完成后重新插入队列（更新时间）
void TimerQueue::ReSetTimer(TimeTask* task)
{
    if(task->isAutoReset()) //如果是自动重启的
    {
        //重新计算时间
        task->updateStamp();
        //插回
        {
            std::lock_guard<std::mutex> lock(_lock);
            _timetasks.insert(pair(reinterpret_cast<uint64_t>(task),task));

        }
    }
}

//超时任务添加到 threadpool 中   
void TimerQueue::handle()
{
    auto now = clock::now();
    {
        std::lock_guard<std::mutex> lock(_lock);
        auto& timeoutqueue = _timeoutqueue;    //超时对了
        for (auto p : _timetasks)
        {
            if (p.second->When() <= now)//超时的
            
                timeoutqueue.push_back(p);  
            else
                break;
        }

        TimeTask* task;

        //删除节点或 autoset
        for(auto p:timeoutqueue)
        {
            task = p.second;
            _timetasks.erase(p.first);
            if(task->isCanceled())
            {
                delete task;
                continue;
            }
            _async_evt_pool.AddTask(task->GetHandle());
            if(task->isAutoReset())
            {
                task->updateStamp();
                _timetasks.insert(p);
            }
            else
                delete task;
        }
        timeoutqueue.clear();
    }
}


/**
 * @brief   将根据超时时间设置一个 timetask 并 添加到定时器队列中
 * 
 * @param ts    超时时间点
 * @param cb    超时回调
 * @param interval  触发间隔
 * @return timetask_t   定时事件句柄
 */
timetask_t TimerQueue::addTimer(Timestamp ts,TimerCallback cb,Millisecond interval)
{
    TimeTask* ptr = new TimeTask(ts,cb,interval);
    timetask_t taskid = reinterpret_cast<uint64_t>(ptr);
    _timetasks.insert(pair(taskid,ptr));

    return taskid;
}


/**
 * @brief 取消本次事件
 * 
 * @param timer 定时事件句柄 
 */
void TimerQueue::cancelTimer(timetask_t timer)       
{
    auto it = _timetasks.find(timer);
    if(it != _timetasks.end())
        it->second->cancel();
}



