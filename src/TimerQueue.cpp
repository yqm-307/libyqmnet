#include <string>
#include <algorithm>
#include <thread>
#include "../include/TimerQueue.h"

using namespace net;

#define Task(xx) (*this)

TimerQueue::TimerQueue(EventLoop* loop)
    :_loop(loop),
    _timer(),
    _lock(),
    _timetasks([](timetask_t a,timetask_t b)->bool{
                return reinterpret_cast<TimeTask*>(a)->interval() < reinterpret_cast<TimeTask*>(b)->interval();
            })
{
    std::thread([this](){
        while(1)
            handle();   
    }).detach();
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

//任务处理程序，执行一次处理一个任务
//如果是重复任务，则需要重新计算 触发时间 并插入队列     
void TimerQueue::handle()
{
    bool t=true;
    while(_timetasks.size() <= 0); //等待任务
    TimeTask* tmp = Front();
    Dequeue();

    while(t){
        if(_timer.TryReset(tmp))    //执行
        {
            t!=t;
            if(tmp->isAutoReset())  //重新入队或释放
                ReSetTimer(tmp);
            else
                delete tmp;
        }
    }
}



//对外接口 添加定时事件
timetask_t TimerQueue::addTimer(Timestamp ts,TimerCallback cb,Millisecond interval)
{
    TimeTask* ptr = new TimeTask(ts,cb,interval);
    timetask_t taskid = reinterpret_cast<uint64_t>(ptr);
    _timetasks.insert(pair(taskid,ptr));

    return taskid;
}


//取消一个定时事件
void TimerQueue::cancelTimer(timetask_t timer)       
{
    auto it = _timetasks.find(timer);
    if(it != _timetasks.end())
        it->second->cancel();
}




void TimerQueue::Dequeue()
{
    std::lock_guard<std::mutex> lock(_lock);
    _timetasks.erase(_timetasks.begin());
}

void TimerQueue::Enqueue(TimeTask* task)
{
    std::lock_guard<std::mutex> lock(_lock);
    _timetasks.insert(pair(reinterpret_cast<uint64_t>(task),task));
}

TimeTask* TimerQueue::Front()
{
    std::lock_guard<std::mutex> lock(_lock);
    return _timetasks.begin()->second;
}