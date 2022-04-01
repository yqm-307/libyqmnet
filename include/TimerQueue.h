#ifndef TIMERQUEUE_H
#define TIMERQUEUE_H
/*
    timerqueue 内置一个 Timer ，顺序执行每个时间戳的cb；
    
    ps：最开始我以为是将所有Timer按照顺序排进队列，但是想了想有点怪，定时器本身就是按照时间触发，怎么还会需要排序呢？
    后来发现是一种节省系统资源的方法，将时间点队列化，然后设置一个定时器，每次执行一次，就从队列中取出下一个时间和cb。
*/
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
