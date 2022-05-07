#ifndef TCPSERVERSINGLE_H
#define TCPSERVERSINGLE_H

#include <unordered_set>
#include "CallBack.h"
#include "Acceptor.h"

namespace net
{
class EventLoop;

class TcpServerSingle:noncopyable
{
public:
    TcpServerSingle(EventLoop* loop,const IPAddress& local);

    void setConnectionCallback(const ConnectionCallback& cb);
    void setMessageCallback(const MessageCallback& cb);
    void setWriteCompleteCallback(const WriteCompleteCallback& cb);

    void start();
private:
    void OnConnection(int connfd,const IPAddress& local,const IPAddress& peer);

    void closeConnection(const TcpConnectionPtr& conn);

    typedef std::unordered_set<TcpConnectionPtr> ConnectionList;

    EventLoop* _loop;
    Acceptor _accept;
    ConnectionList _connections;
    
    ConnectionCallback _connectioncb;
    MessageCallback _msgcb;
    WriteCompleteCallback _writeCompletecb;
};

}

#endif