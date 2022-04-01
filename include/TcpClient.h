#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <memory>
#include "CallBack.h"
#include "Connector.h"
#include "Timer.h"
#include "noncopyable.h"

namespace net
{

class TcpClient:noncopyable
{
public:

    TcpClient(EventLoop* loop,const IPAddress& peer);
    ~TcpClient();

    void start();
    void setConnectionCallback(const ConnectionCallback& cb)
    { _connectioncb = cb;}
    void setMessageCallback(const MessageCallback& cb)
    { _msgcb = cb;}
    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    { _writeCompletecb = cb;}
    void setErrorCallback(const ErrorCallback& cb)
    { _connector->setErrorCallback(cb);}
private:
    void retry();
    void OnConnection(int connfd,const IPAddress& local,const IPAddress& peer);
    void closeConnection(const TcpConnectionPtr& conn);
public:
    typedef std::unique_ptr<Connector> ConnectorPtr;

    EventLoop* _loop;
    bool _connected;
    const IPAddress _peer;
    TimeTask* _trytimer;
    ConnectorPtr _connector;
    TcpConnectionPtr _connection;
    ConnectionCallback _connectioncb;
    MessageCallback _msgcb;
    WriteCompleteCallback _writeCompletecb;
};

}



#endif