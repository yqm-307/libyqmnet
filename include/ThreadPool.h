#ifndef THREADPOOL_H
#define THREADPOOL_H
/*
    线程池工作流程：
		池中所有线程都是一个这样的逻辑：不断从taskqueue中取出一个task并执行，如果taskqueue空，则线程休眠。
	线程挂起条件：
		消息队列为空，线程挂起
	阻塞线程的唤醒：
		在插入task的时候，会检测是否有阻塞的线程，如果有就唤醒一个。如果都在运行，就直接入队。
	
*/
#include <functional>
#include <vector>
#include <thread>
#include <atomic>
#include <queue>
#include <condition_variable>   //cv
#include <cassert>
#include "CallBack.h"
#include "Thread.h"		//封装线程
#include "Config.h"
#include "Logger.h"


namespace Util{

enum ThreadPoolErrnoCode{
	PoolStop=0,
	TaskQueueFull,
	TaskQueueBlock,
	Success
};

template<typename TaskFunc>
class ThreadPool
{
public:
	ThreadPool(int thnum,int maxqueuesize = 65535,const net::ThreadInitCallback& cb=nullptr)
		:_threadnum(thnum),
		_initqueuesize(maxqueuesize),
		_threads(thnum,nullptr),
		_pool_is_in_run(true)
	{
		for(int i=0;i<_threadnum;++i)
		{
			_threads[i] = new Thread();
			Thread* ptr = _threads[i];
			//执行任务线程
			ptr->Start(std::bind([this](Thread* ptr)->ThreadStatus{
				if(!_pool_is_in_run)
					return Stop;
				{//进入临界区
					net::lock_guard<net::Mutex> lock(_lock);
					if(_taskqueue.empty())	//没有任务
					{
						_freethreads.push(ptr);
						return Blocking;
					}
					else{	//队列非空，取出任务执行
						(_taskqueue.front())();	//task
						_taskqueue.pop();
					}
				}
				return Running;	
			},ptr));
		}
	}
	//线程池析构并不是热点操作，所以没有性能要求
	~ThreadPool()
	{
		stop();
		TRACE("queuetask %d\nfree thread: %d",_taskqueue.size(),_freethreads.size());

		for(auto ptr : _threads)
		{
			delete ptr;
		}
	}

	//添加一个task
	ThreadPoolErrnoCode AddTask(const TaskFunc& task)
	{
		if(_pool_is_in_run)	//线程池在运行中
		{
			{//进入临界区
				net::lock_guard<net::Mutex> lock(_lock);
				if(_taskqueue.size() >= _initqueuesize)	//超出任务数
				{
#if(YNET_THREADPOOL_RUN_IN_MAIN_THREAD)
					task();
					return Success;
#else
					return TaskQueueFull;

#endif
				}

				_taskqueue.push(task);	//插入任务队列
				if(!_freethreads.empty())
					WakeUpOne();
			}
			return Success;
		}
		return PoolStop;	//线程池停止运行
	}
	ThreadPoolErrnoCode AddTask(TaskFunc&& task)
	{
		if(_pool_is_in_run)	//线程池在运行中
		{
			{//进入临界区
				net::lock_guard<net::Mutex> lock(_lock);
				if(_taskqueue.size() >= _initqueuesize)	//超出任务数
				{
#if(YNET_THREADPOOL_RUN_IN_MAIN_THREAD)
					task();
					return Success;
#else
					return TaskQueueFull;

#endif
				}
				
				_taskqueue.push(task);	//插入任务队列
				if(!_freethreads.empty())
					WakeUpOne();
			}
			return Success;
		}
		return PoolStop;	//线程池停止运行
	}

	int RunThreadNum()
	{ return (_threadnum - _freethreads.size()); }
	int TaskNum()
	{
		net::lock_guard<net::Mutex> lock(_lock);
		return _taskqueue.size();
	}
private:

	//终止线程池
	void stop()
	{
		_pool_is_in_run = false;	//拒绝新的task

		for (size_t i = 0; i < _threadnum;)	//等待所有thread完成当前任务
		{
			if(!_threads[i]->isRun())	//线程已经stop，下一个
				++i;
			else if(_threads[i]->isBlock())	//线程阻塞，唤醒
			{
				_threads[i]->ReStart();
			}
		}
	}


	/**
	 * @brief 线程调度 当前是遍历，会出现严重的饥饿现象
	 * @return true 
	 * @return false 
	 */
	bool WakeUpOne()
	{
		if(_freethreads.empty())
			return false;	//唤醒失败
		_freethreads.front()->ReStart();
		_freethreads.pop();
		return true;
	}

	const int _threadnum;					//初始线程数量
	const int _initqueuesize;				//初始队列长度，不是硬性的
	typedef std::vector<Thread*> ThreadList;	//加锁
	std::atomic_bool _pool_is_in_run;		//是否正在运行
	ThreadList _threads;					//线程
	std::queue<TaskFunc> _taskqueue;		//任务队列
	net::Mutex _lock;
	std::queue<Thread*> _freethreads;			//空闲队列	用作线程调度
};

}



#endif


