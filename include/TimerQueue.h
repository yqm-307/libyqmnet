#ifndef TIMERQUEUE_H
#define TIMERQUEUE_H



#include <vector>
#include <set>
#include <queue>

#include "Timer.h"
#include "TimeTask.h"
#include "Channel.h"

namespace net
{
class TimerQueue
{
public:
    TimerQueue(EventLoop* loop);
    ~TimerQueue();

    TimeTask* addTimer(Timestamp ts,TimerCallback cb,Millisecond interval=-1ms);     //添加一个新事件
    void cancelTimer(TimeTask* timer);                     //取消一个计时器，就直接找到，然后弹出
    void cnacelTimer(TimeTask&& timer);
private:
    void addTimer(TimeTask* task);
    //std::vector<TimeTask> Get_Expired();       //获取失效的事件
    void handle();
    void ReSetTimer(TimeTask* task);
private:
    void Enqueue(TimeTask* task);
    void Dequeue();
    TimeTask* Top();
private:
    EventLoop* _loop;
    Timer _timer;
    //Channel _channel;
    std::priority_queue<TimeTask*,std::vector<TimeTask*>,std::function<bool(TimeTask*,TimeTask*)>> 
            _timerqueue;   //消费者生产者队列——需要加锁：加锁时间只有出队入队可以尝试mutex和spin
    std::mutex _lock;
};

}

#endif
