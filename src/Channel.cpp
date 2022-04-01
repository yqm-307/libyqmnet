#include <assert.h>

#include "../include/Channel.h"
#include "../include/EventLoop.h"


using namespace net;

Channel::Channel(EventLoop* loop,int fd)
    :polling(false),
    _loop(loop),
    _fd(fd),
    _handlingEvents(false),
    _tied(false),
    events_(0),
    revents_(0)
{
}

Channel::~Channel()
{ assert(!_handlingEvents);}


//处理事件回调
void Channel::doTask()
{
    _loop->assertInLoopThread();
    if(_tied)
    {
        auto guard = _tie.lock();
        if(guard != nullptr)
            handleEventsGuard();
    }
    else 
        handleEventsGuard();
}

void Channel::handleEventsGuard()
{
    _handlingEvents = true;
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
        if (_closecallback) _closecallback();
    }
    if (revents_ & EPOLLERR) {
        if (_errorcallback) _errorcallback();
    }
    if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        if (_readcallback) _readcallback();
    }    
    if (revents_ & EPOLLOUT) {
        if (_writecallback) _writecallback();
    }
    _handlingEvents = false;
}

//通过eventloop更新到epoller中
void Channel::update()
{
    _loop->updateChannel(this);
}

//删除自己
void Channel::remove()
{
    _loop->removeChannel(this);
}


//转化为weak_ptr，管理生命期
void Channel::tie(const std::shared_ptr<void>& obj)
{
    _tie = obj;
    _tied = true;
}
