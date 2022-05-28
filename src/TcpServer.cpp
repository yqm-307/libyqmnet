#include <memory>
#include <condition_variable>

#include "../include/Logger.h"
#include "../include/TcpConnection.h"
#include "../include/TcpServerSingle.h"
#include "../include/EventLoop.h"
#include "../include/TcpServer.h"
#include "../include/CallBack.h"



using namespace net;

TcpServer::TcpServer(EventLoop* loop, const IPAddress& local)
        :_baseLoop(loop),
        _threadnums(1),
        _state(false),
        _local(local),
        _threadInitCallback(defaultThreadInitCallback),
        _messageCallback(defaultMessageCallback),
        _connectionCallback(defaultConnectionCallback)
{
    INFO("create TcpServer() %s", local.GetIPPort().c_str());
}

TcpServer::~TcpServer()
{
    for(auto& a:_loops)
        if(a!=nullptr)
            a->sleep();
    for(auto& a: _threads)
        a->join();
    TRACE("~TcpServer()");
}


//线程数量
void TcpServer::setNumThread(size_t n)
{
    _baseLoop->assertInLoopThread();
    assert(n>0);
    assert(!_state);
    _threadnums = n;
    _loops.resize(n);
}


//启动所有eventloop线程，就是在主EventLoop中执行启动所有 subEventLoop
void TcpServer::start()
{
    if(_state.exchange(true))
        return;
    _baseLoop->addTask([=](){startInLoop();});
}

//开启所有EventLoop
void TcpServer::startInLoop()
{
    INFO("TcpServer::start() %s with %lu eventloop thread(s)",
        _local.GetIPPort().c_str(),_threadnums);

    _baseServer = std::make_unique<TcpServerSingle>(_baseLoop,_local);
    _baseServer->setConnectionCallback(_connectionCallback);
    _baseServer->setMessageCallback(_messageCallback);
    _baseServer->setWriteCompleteCallback(_writeCompleteCallback);
    if(_decode) _baseServer->setDeCode(_decode);
    if(_encode) _baseServer->setEnCode(_encode);
    _threadInitCallback(0);
    _baseServer->start();   //主EventLoop 开启监听

    for(size_t i=1;i<_threadnums;++i)   //从属EventLoop
    {
        auto thread = new std::thread(std::bind(
            &TcpServer::runInThread,this,i));
            
        {
            net::lock_guard<Mutex> lock(_mutex);
            while(_loops[i] == nullptr)
                _cond.wait();
        }
        _threads.emplace_back(thread);
    }
}


void TcpServer::runInThread(size_t index)
{
    EventLoop loop;
    TcpServerSingle server(&loop,_local);

    server.setConnectionCallback(_connectionCallback);
    server.setMessageCallback(_messageCallback);
    server.setWriteCompleteCallback(_writeCompleteCallback);

    {
        net::lock_guard<Mutex> lock(_mutex);
        _loops[index] = &loop;
        _cond.notify_one(); //唤醒一个
    }

    _threadInitCallback(index);
    server.start();
    loop.loop();
    _loops[index] = nullptr;
}

