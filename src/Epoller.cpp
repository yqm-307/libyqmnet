#include <sys/epoll.h>
#include <unistd.h>
#include <assert.h>
#include "../include/Logger.h"
#include "../include/EventLoop.h"

using namespace net;


Epoller::Epoller(EventLoop* loop)
    :_loop(loop),
    _epfd(epoll_create(8)),
    _events(8)
{
}

Epoller::~Epoller()
{
    close(_epfd);
}


//调用一次epoll_wait获取事件
void Epoller::poll(ChannelList& Channels)    
{
    _loop->assertInLoopThread();

    size_t num = _events.size();
    ssize_t n=0;
    if( -1 >= (n = epoll_wait(_epfd,_events.data(),num,0)))
        ERROR("Epoller::poll() error");
    
    for(int i=0;i<n;++i)
    {
        epoll_event evt = _events[i];

        auto channel = static_cast<Channel*>(evt.data.ptr);
        channel->setRevents(evt.events);
        Channels.push_back(channel);
    }

    if(n == num)    //epoll_wait不会有插入操作，所以需要手动扩容
        _events.resize(2*num);  //慢启动
}

//将一个时间的channel插入到_events中
void Epoller::updateChannel(Channel* channel)       
{
    //int op = channel->events(); //获取 epoll_events
    assert(_loop->isInLoopThread());

    int op=0;
    if (!channel->polling) {    //第一次添加进epoll表
        assert(!channel->isNoneEvents());
        op = EPOLL_CTL_ADD;
        channel->polling = true;
    }
    else if (!channel->isNoneEvents()) {    //修改需要关注的事件
        op = EPOLL_CTL_MOD;
    }
    else {  //是删除事件
        op = EPOLL_CTL_DEL;         
        channel->polling = false;
    }
    updateChannel(op,channel);
}

void Epoller::updateChannel(int op, Channel* channel)
{
    struct epoll_event event;
    event.events = channel->events();
    event.data.ptr = channel;
    
    int ret = epoll_ctl(_epfd,op,channel->GetFd(),&event);
    if( ret == -1)
        ERROR("Epoller::updateChannel  epoll_ctl() is wrong!");
}