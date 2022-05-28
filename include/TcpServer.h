#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

#include "TcpServerSingle.h"
#include "IPAddress.h"
#include "CallBack.h"
#include "noncopyable.h"
#include "MyLocker.h"

namespace net
{
class EventLoop;
class TcpServerManager;
class IPAddress;
class TcpServer: noncopyable
{
public:
    TcpServer(EventLoop* loop,const IPAddress& local);
    ~TcpServer();

    void setNumThread(size_t n);

    void start();
    void setThreadInitCallback(const ThreadInitCallback& cb)
    { _threadInitCallback = cb; }
    void setConnectionCallback(const ConnectionCallback& cb)
    { _connectionCallback = cb; }
    void setMessageCallback(const MessageCallback& cb)
    { _messageCallback = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    { _writeCompleteCallback = cb;}
    void setEnCode(const EnCodeFunc& cb)
    { _encode = cb; }
    void setDeCode(const DeCodeFunc& cb)
    { _decode = cb; }

private:
    void startInLoop();
    void runInThread(size_t index);

    typedef std::unique_ptr<std::thread> ThreadPtr;
    typedef std::vector<ThreadPtr> ThreadPtrList;
    typedef std::unique_ptr<TcpServerSingle> TcpServerSinglePtr;
    typedef std::vector<EventLoop*> EventLoopList;

    EventLoop* _baseLoop;   //主循环
    TcpServerSinglePtr _baseServer;
    ThreadPtrList _threads;
    EventLoopList _loops;
    net::Mutex _mutex;
    size_t _threadnums;
    std::atomic_bool _state;
    IPAddress _local;
    net::Sem_t _cond;

    ThreadInitCallback _threadInitCallback;
    ConnectionCallback _connectionCallback;
    MessageCallback _messageCallback;
    WriteCompleteCallback _writeCompleteCallback;
    DeCodeFunc _decode;
    EnCodeFunc _encode;
};
}


#endif

