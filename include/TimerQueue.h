#ifndef TIMERQUEUE_H
#define TIMERQUEUE_H



#include <vector>
#include <map>
#include <queue>

#include "Config.h"
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
    void handle();
    void ReSetTimer(TimeTask* task);
private:
    EventLoop* _loop;
    Timer _timer;
    Util::ThreadPool<TimerCallback> _async_evt_pool;    //异步事件池
    std::vector<std::pair<timetask_t,TimeTask*>> _timeoutqueue;
    std::map<timetask_t,TimeTask*,std::function<bool(timetask_t,timetask_t)>> _timetasks;
    std::mutex _lock;
};

}

#endif
