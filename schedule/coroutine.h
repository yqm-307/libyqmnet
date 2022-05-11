/**
 * @file coroutine.h
 * @author your name (you@domain.com)
 * @brief 封装协程
 * @version 0.1
 * @date 2022-05-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <iostream>

#include <libco/co_routine.h>
#include <memory>
#include <shared_mutex>

namespace net
{

class Coroutine:std::enable_shared_from_this<Coroutine>
{
public:
    typedef std::shared_ptr<Coroutine> CoroutinePtr;

private:
    
};

}