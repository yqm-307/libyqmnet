#include "../include/EventLoop.h"
#include "../include/Timer.h"
#include "../include/TimerQueue.h"
#include "../include/Epoller.h"
#include "../include/Logger.h"

#include <unistd.h>
#include <assert.h>
#include <signal.h>
#include <thread>
#include <sys/eventfd.h>    //eventfd进程间通信
#include <sys/epoll.h>
#include <fcntl.h>
using namespace net;

//仅内部可见
namespace
{
//TLS线程局部变量
thread_local EventLoop* t_Eventloop = nullptr;

//获取系统线程tid   线程一般有两个控制句柄，一个是线程描述符（操作系统内唯一）、另一个就是进程内线程描述符（单个进程内唯一，但是可能出现不同进程有相同的线程id）
int Gettid()
{
    return gettid();
}


//忽略掉信号SIGPIPE
class IgnoreSigPipe
{
public:
    IgnoreSigPipe()
    {
        signal(SIGPIPE,SIG_IGN);
    }
}ignore;

}



EventLoop::EventLoop()
    :_tid(Gettid()),
    _timerQueue(this),
    _sleeping(false),
    _doingPendingTasks(false),
    _poller(this),
    _wakeupFd(::eventfd(0,EFD_CLOEXEC | EFD_NONBLOCK)),
    _wakeupChannel(this,_wakeupFd)
{   
    if(_wakeupFd == -1)
        FATAL("EventLoop::eventfd() error");
    
    _wakeupChannel.setReadCallback([this](){handleRead();});
    _wakeupChannel.enableRead();

    assert(t_Eventloop == nullptr);
    t_Eventloop = this;
}

EventLoop::~EventLoop()
{
    assert(t_Eventloop == this);
    t_Eventloop == nullptr;
}


//将task加入taskqueue
void EventLoop::addTask(const Task& task)
{
    if(isInLoopThread())    //如果此线程就是循环线程，则直接执行
    {
        task(); 
        return;
    }
    else
        addInQueue(task);
}

//将task加入taskqueue   (右值版)
void EventLoop::addTask(Task&& task)
{
    if(isInLoopThread())    //如果此线程就是循环线程，则直接执行
    {
        task(); 
        return;
    }
    else
        addInQueue(std::move(task));

}

void EventLoop::addInQueue(const Task& task)
{
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _pendingTasks.push_back(task);
    }
    if(!isInLoopThread() || _doingPendingTasks)
        wakeup();
}

void EventLoop::addInQueue(Task&& task)
{
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _pendingTasks.push_back(std::move(task));
    }
    if(!isInLoopThread() || _doingPendingTasks)
        wakeup();
}

//发送消息唤醒
void EventLoop::wakeup()
{
    uint64_t one;
    ssize_t n = write(_wakeupFd,&n,sizeof(one));

    if(n!=sizeof(one))
        ERROR("EventLoop::wakeup() should ::write() %lu bytes",sizeof(one));
}

void EventLoop::loop()
{
    assertInLoopThread();
    _sleeping = false;
    TRACE("EventLoop %p poll", this);
    while(!_sleeping)
    {
        _channels.clear();
        _poller.poll(_channels);    //从epoll 中获取的读写事件
        for(auto p : _channels)
            p->doTask();
        doPendingTasks();
    } 
    TRACE("EventLoop %p quit", this);
}

void EventLoop::sleep()
{
    assert(!_sleeping);
    _sleeping = true;   //休眠中
    if(!isInLoopThread())   
        wakeup();
}

timetask_t EventLoop::runAt(Timestamp when, TimerCallback callback)
{
    return _timerQueue.addTimer(std::move(when),std::move(callback));
}

timetask_t EventLoop::runAfter(Nanosecond interval, TimerCallback callback)
{
    return _timerQueue.addTimer(clock::now() + interval,std::move(callback));
}

timetask_t EventLoop::runEvery(Nanosecond interval,TimerCallback callback)
{
    return _timerQueue.addTimer(clock::now() + interval,std::move(callback),Millisecond(interval.count() /1000/1000));
}

void EventLoop::cancelTimer(timetask_t timeid)
{
    _timerQueue.cancelTimer(timeid);
}


//在poller中 插入、修改或者删除
void EventLoop::updateChannel(Channel* channel)
{
    assertInLoopThread();
    _poller.updateChannel(channel); 
}

//删除一个channel，因为是指针，所以会修改这块内存，直接注销所有事件，大家都用不了了
void EventLoop::removeChannel(Channel* channel)
{
    assertInLoopThread();
    channel->disableAll();  //注销所有事件
}

//是不是当前线程
bool EventLoop::isInLoopThread()
{
    return _tid == Gettid();
}

void EventLoop::assertInLoopThread()
{
    assert(isInLoopThread());
}

void EventLoop::assertNotInLoopThread()
{
    assert(!isInLoopThread());
}

void EventLoop::handleRead()
{
    uint64_t one;
    ssize_t n = read(_wakeupFd,&one,sizeof(one));
    if(n!=sizeof(one))
        ERROR("EventLoop::handleRead() should ::read() %lu bytes",sizeof(one));
}


/*
    将待处理task取出并处理。

        双队列：两个队列一个只在这个函数中存在不存在线程同步问题，另一个队列是在多线程环境下的需要加锁
    原本需要将每个任务取出并执行，然后退出，整个过程都是临界区。但是我们通过交换给临时队列，可以让临界
    区缩短为只有swap，代价就是消耗额外内存
*/
void EventLoop::doPendingTasks()
{
    assertInLoopThread();
    std::vector<Task> tasks;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        tasks.swap(_pendingTasks);
    }
    _doingPendingTasks = true;
    for(auto p : tasks)
        p();
    _doingPendingTasks = false;
}
