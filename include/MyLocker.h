#ifndef MYLOCKER_H
#define MYLOCKER_H

#include <pthread.h>
#include <atomic>
#include <thread>
#include <Logger.h>
#include <noncopyable.h>

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

class Mutex
{
public:
	Mutex():_mutex(PTHREAD_MUTEX_INITIALIZER){}
	~Mutex(){pthread_mutex_destroy(&_mutex);}
	void lock()
	{
		if(0>pthread_mutex_lock(&_mutex))
		{
			FATAL("Mutex::lock() error!");
			exit(-1);
		}
	}
	void unlock()
	{
		if(0>pthread_mutex_unlock(&_mutex))
		{
			FATAL("Mutex::unlock() error!");
			exit(-1);
		}
	}

private:
	pthread_mutex_t _mutex;
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


class Sem_t:noncopyable
{
public:
	Sem_t()
		:_mutex(PTHREAD_MUTEX_INITIALIZER),
		_cond_t(PTHREAD_COND_INITIALIZER){}
	~Sem_t()
	{
		pthread_mutex_destroy(&_mutex);
		pthread_cond_destroy(&_cond_t);
	}

	void wait()
	{
		if( 0 > pthread_cond_wait(&_cond_t,&_mutex))
		{
			FATAL("MyLocker::sem_t::wait() error!");
			exit(-1);
		}
	}
	void notify_one()
	{
		if(0>pthread_cond_signal(&_cond_t))
		{
			FATAL("sem_t::notify_one() error!");
			exit(-1);
		}
	}

	void notify_all()
	{
		if(0>pthread_cond_broadcast(&_cond_t))
		{
			FATAL("sem_t::notify_all() error!");
			exit(-1);
		}
	}

private:
	pthread_mutex_t _mutex;
	pthread_cond_t _cond_t;
};



class CountDownLatch:noncopyable
{
public:
	CountDownLatch(int cot):_count(cot){}
	~CountDownLatch()
	{
		if(_count!=0)
			DEBUG("~CountDownLatch() _count=%d",_count);
	}

	void wait()
	{
		lock_guard<Mutex> lock(_lock);
		while(_count > 0)
			_sem.wait();
	}	
	//todo 设置超时的阻塞时间
	//void clockwait(Timestamp);
	void down()
	{
		lock_guard<Mutex> lock(_lock);
		--_count;
		if(_count <= 0)
			_sem.notify_all();
	}
private:
	int _count;
	Mutex _lock;
	Sem_t _sem;
};


}



#endif