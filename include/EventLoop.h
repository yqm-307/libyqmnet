#ifndef EVENTLOOP_H
#define EVENTLOOP_H
/*
        EventLoop 是网络库的核心处理单元。所有事件网络事件和用户回调都会在这一个io线程中被处理。
    采用的是 one loop one thread ，每个循环一个线程。这样做法简单高效，不易出现复杂的线程同步问题。少加锁性能高
*/
#include <atomic>
#include <mutex>
#include <vector>
#include <sys/types.h>


#include "Epoller.h"
#include "TimerQueue.h"
#include "MyLocker.h"

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
    
    /**
     * @brief 添加 io 任务，需要在主线程中循环，有严格的次序要求
     * @param task 
     */
    void addTask(const Task& task);
    void addTask(Task&& task);
    /**
     * @brief 其他类型任务，可在任何线程中循环，可封装线程池
     * @param task 
     */
    void addInQueue(const Task& task);
    void addInQueue(Task&& task);

    /**
     * @brief 定时任务
     * @param when 触发时间戳
     * @param callback 回调事件
     * @return TimeTask* 定时任务句柄
     */
    timetask_t runAt(Timestamp when, TimerCallback callback);
    timetask_t runAfter(Nanosecond interval, TimerCallback callback);
    timetask_t runEvery(Nanosecond interval, TimerCallback callback);
    void cancelTimer(timetask_t timeid);

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
    Mutex _mutex;
    std::vector<Task> _pendingTasks;    //加锁

    TimerQueue _timerQueue;             

};

}

#endif