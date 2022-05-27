#include <include/Thread.h>
#include <include/EventLoop.h>


class LoopThread
{

public:
    LoopThread()
        :_loop(nullptr)
    {}

    net::EventLoop* start()
    {
        _thread.Start(std::bind(&LoopThread::runInLoop,this));
        
        return _loop;
    }
private:

    Util::ThreadStatus runInLoop()
    {


        return Util::Stop;
    }

private:
    net::EventLoop* _loop;
    Util::Thread _thread;

};