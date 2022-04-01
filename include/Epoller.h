#ifndef EPOLLER_H
#define EPOLLER_H

#include <vector>
#include "noncopyable.h"


namespace net
{

class EventLoop;
class Channel;

class Epoller : noncopyable
{
public:
    typedef std::vector<Channel*> ChannelList;

    Epoller(EventLoop* loop);                  
    ~Epoller();


    void poll(ChannelList& activeChannels);     //调用一次epoll_wait 分发事件
    void updateChannel(Channel* channel);       //将一个时间的channel插入到_events中

private:
    void updateChannel(int event_option,Channel* channel);
    
    EventLoop* _loop;
    std::vector<struct epoll_event> _events;
    int _epfd;
    
};

}


#endif