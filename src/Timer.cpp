#include "../include/Timer.h"
#include <iostream>
#include <stdio.h>
#include <assert.h>

using namespace net;
using namespace clock;

Timer::Timer(Millisecond tick,TickCallbcak cb)
    :_time(tick),
    _task(cb),
    _running(false)
{
}


void Timer::Stop()
{
    if(_running)
        _running.store(false);
    std::this_thread::sleep_for(_time);
}

void Timer::Start()
{
    std::thread([this](){
        _running.store(true);
        while(_running){ 
            std::this_thread::sleep_for(_time);// tick        
            _task();    //do task
        }
    }).detach();
}
