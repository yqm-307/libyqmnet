#ifndef CHANNEL_H
#define CHANNEL_H

#include <functional>
#include <sys/epoll.h>
#include <memory>
#include "noncopyable.h"

namespace net{

class EventLoop;

class Channel: noncopyable
{


public:
    Channel(EventLoop* loop, int fd);
    ~Channel();

    typedef std::function<void()> ReadCallback;
    typedef std::function<void()> WriteCallback;
    typedef std::function<void()> CloseCallback;
    typedef std::function<void()> ErrorCallback;
    bool polling;
    bool isReading()
    { return events_ & EPOLLIN;}
    bool isWriting()
    { return events_ & EPOLLOUT;}
    
    //设置回调
    void setReadCallback(ReadCallback read)
    { _readcallback = read; }
    void setWriteCallback(WriteCallback write)
    { _writecallback = write; }
    void setCloseCallback(CloseCallback close)
    { _closecallback = close; }
    void setErrorCallbak(ErrorCallback error)
    { _errorcallback = error; }
    void setRevents(unsigned revents)
    { revents_ = revents; }


    //epollctl option
    //注册可读事件
    void enableRead()
    { events_ |= (EPOLLIN | EPOLLPRI); update();}
    //注册可写事件
    void enableWrite()
    { events_ |= EPOLLOUT; update();}
    //注销可读事件
    void disableRead()
    { events_ &= ~EPOLLIN; update(); }
    //注销可写事件
    void disableWrite()
    { events_ &= ~EPOLLOUT; update();}
    //注销所有事件
    void disableAll()
    { events_ = 0; update();}

    //将事件取出调用回调
    void doTask();
    
    bool isNoneEvents() const
    { return events_ == 0; }    
    unsigned events() const
    { return events_; }
    int GetFd() const
    { return _fd;}

    void handleEventsGuard();
    void tie(const std::shared_ptr<void>& obj);
private:
    void update();
    void remove();

    EventLoop* _loop;
    int _fd;

    std::weak_ptr<void> _tie;
    bool _tied;

    unsigned events_;   //事件类型
    unsigned revents_;  //epoll_ctl 事件

    bool _handlingEvents;

    //回调事件
    ReadCallback _readcallback;
    WriteCallback _writecallback;
    CloseCallback _closecallback;
    ErrorCallback _errorcallback;
};


}
#endif