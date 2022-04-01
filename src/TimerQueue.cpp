#include <string>
#include <algorithm>
#include <thread>
#include "../include/TimerQueue.h"

using namespace net;


TimerQueue::TimerQueue(EventLoop* loop)
    :_loop(loop),
    _timer(),
    _lock(),
    _timerqueue([](TimeTask* a,TimeTask* b){
                return a->interval()<b->interval();
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
    while(_timerqueue.size()>0) //全部析构
    {
        ptr = _timerqueue.top();
        _timerqueue.pop();
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
            _timerqueue.push(task);
        }
    }
}

//任务处理程序，执行一次处理一个任务
//如果是重复任务，则需要重新计算when并插入队列      此处可以优化?  需要出队再入队，不过resettime后也需要再平衡和重新插入差不多了（本身其实也是个链式结构--树）
void TimerQueue::handle()
{
    bool t=true;
    while(_timerqueue.size() <= 0); //等待任务
    TimeTask* tmp = Top();
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
TimeTask* TimerQueue::addTimer(Timestamp ts,TimerCallback cb,Millisecond interval)
{
    TimeTask* ptr = new TimeTask(ts,cb,interval);
    addTimer(ptr);
    return ptr;
}

void TimerQueue::addTimer(TimeTask* task)
{
    _timerqueue.push(task);
}

//取消一个定时事件
void TimerQueue::cancelTimer(TimeTask* timer)       
{
    timer->cancel();
}
void TimerQueue::cnacelTimer(TimeTask&& timer)
{
    timer.cancel();
}




void TimerQueue::Dequeue()
{
    std::lock_guard<std::mutex> lock(_lock);
    _timerqueue.pop();
}

void TimerQueue::Enqueue(TimeTask* task)
{
    std::lock_guard<std::mutex> lock(_lock);
    _timerqueue.push(task);
}

TimeTask* TimerQueue::Top()
{
    std::lock_guard<std::mutex> lock(_lock);
    return _timerqueue.top();
}