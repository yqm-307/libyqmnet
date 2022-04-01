#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <atomic>
#include <thread>

namespace net
{

template<typename _Lock>
class lock_guard
{
public:
	lock_guard(_Lock& a):_lock(a)
	{
		a.lock();
	}
	
	~lock_guard()
	{
		_lock.unlock();
	}
private:
	_Lock& _lock;
};

//自旋锁
class Spinlock
{
public:
	Spinlock():
		flag(ATOMIC_FLAG_INIT){}
    
	
    void lock()
	{
		while (flag.test_and_set(std::memory_order_acquire));
	}
	
    void unlock()
	{
		flag.clear(std::memory_order_release);
	}

private:
	std::atomic_flag flag;
};

}



#endif