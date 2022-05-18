#ifndef TIMERQUEUE_H
#define TIMERQUEUE_H



#include <vector>
#include <map>
#include <queue>

#include "Timer.h"
#include "TimeTask.h"
#include "Channel.h"
#include "ThreadPool.h"

namespace net
{
class TimerQueue:std::enable_shared_from_this<TimerQueue>
{
public:
    typedef std::pair<timetask_t,TimeTask*> pair;
    TimerQueue(EventLoop* loop);
    ~TimerQueue();

    timetask_t addTimer(Timestamp ts,TimerCallback cb,Millisecond interval=-1ms);     //添加一个新事件
    void cancelTimer(timetask_t);                     //取消一个计时器，就直接找到，然后弹出

private:
    //std::vector<TimeTask> Get_Expired();       //获取失效的事件
    void handle();
    void ReSetTimer(TimeTask* task);
private:
    EventLoop* _loop;
    Timer _timer;
    Util::ThreadPool<TimerCallback> _async_evt_pool;    //异步事件池
    std::vector<std::pair<timetask_t,TimeTask*>> _timeoutqueue;
    //Channel _channel;
    std::map<timetask_t,TimeTask*,std::function<bool(timetask_t,timetask_t)>> _timetasks;
    //std::priority_queue<timetask_t,std::vector<TimeTask*>,std::function<bool(TimeTask*,TimeTask*)>>         _timerqueue;   //消费者生产者队列——需要加锁：加锁时间只有出队入队可以尝试mutex和spin
    std::mutex _lock;
};

}

#endif
