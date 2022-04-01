#ifndef EVENTLOOP_H
#define EVENTLOOP_H
/*
        EventLoop 是网络库的核心处理单元。所有事件网络事件和用户回调都会在这一个io线程中被处理。
    muduo 采用的是 one loop one thread ，每个循环一个线程。这样做法简单高效，不易出现线程同步问题。少加锁性能高

        eventloop有两个任务处理队列，一个积攒的是待处理任务，外部可见，供外部插入任务；内部是一个任务队列，
    内部的逻辑是不停拷出外部队列，然后处理，循环此步骤。所以拷贝过程需要加锁
*/

#include <atomic>
#include <mutex>
#include <vector>
#include <sys/types.h>

#include "Timer.h"
#include "Epoller.h"
#include "TimerQueue.h"

namespace net
{

class EventLoop:noncopyable
{
public:
    EventLoop();
    ~EventLoop();

    //主循环
    void loop();
    void sleep();   
    void wakeup();
    
    //添加任务
    void addTask(const Task& task);
    void addTask(Task&& task);

    //
    TimeTask* runAt(Timestamp when, TimerCallback callback);
    TimeTask* runAfter(Nanosecond interval, TimerCallback callback);   
    TimeTask* runEvery(Nanosecond interval, TimerCallback callback);  
    void cancelTimer(TimeTask* timer);

    //channel管理
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);


    //工具函数
    void assertInLoopThread();
    void assertNotInLoopThread();
    bool isInLoopThread();

private:
    void doPendingTasks();
    void handleRead();

    int _tid;               //当前线程的唯一标识


    std::atomic<bool> _sleeping;        //是否正在等待任务
    bool _doingPendingTasks;            //是否正在处理任务
    Epoller _poller;        
    Epoller::ChannelList _channels;
    const int _wakeupFd;            //通过signal机制，唤醒自己
    Channel _wakeupChannel; 
    std::mutex _mutex;
    std::vector<Task> _pendingTasks;    //加锁

    TimerQueue _timerQueue;             

};

}

#endif