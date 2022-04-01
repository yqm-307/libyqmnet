#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <functional>

#include "IPAddress.h"
#include "Channel.h"
#include "noncopyable.h"

/*
    给客户端用的connector
    功能：建立连接，连接回调
*/

namespace net
{
class EventLoop;
class InetAddress;

class Connector
{
public:
    Connector(EventLoop* ,IPAddress );
    ~Connector();

    void connect();
    void setErrorCallback(ErrorCallback cb)
    {   _errorcb = cb; }
    void setOnConnectCallback(OnConnectCallback cb)
    {   _newconnectcb = cb; }
private:

    void handleWrite();


    EventLoop* _loop;
    IPAddress _addr;
    const int _sockfd;
    bool _connected;
    bool _started;
    Channel _channel;
    OnConnectCallback _newconnectcb;
    ErrorCallback _errorcb;

};

}

#endif