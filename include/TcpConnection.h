#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H
#include "noncopyable.h"
#include "Buffer.h"
#include "CallBack.h"
#include "Channel.h"
#include "IPAddress.h"




namespace net
{

class EventLoop;

//enable_shared_from_this作用：
class TcpConnection: noncopyable,
                public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop* loop,int sockfd,
                const IPAddress& local,
                const IPAddress& peer);

    ~TcpConnection();

    void setMessageCallback(const MessageCallback& cb)
    { _msgcb = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    { _writecompletecb = cb; }
    void setHighWaterMarkVallback(const HighWaterMarkCallback& cb, size_t mark)
    { _highwatermarkcallback = cb; _highWaterMark = mark;}

    //内部使用
    void setCloseCallback(const CloseCallback& cb)
    { _closecb = cb;}

    //单个tcp服务器
    void connectBuildOver();
    
    const IPAddress& local() const
    { return _local;}
    const IPAddress& peer() const
    { return _peer;}
    bool connected() const;
    bool disconnected() const;



    //线程安全的io操作
    void send(const char* data,size_t len);
    void shutdown();
    void forceClose();


    //其他操作
    void stopRead();
    void startRead();
    bool isReading() // not thread safe
    { return _channel.isReading(); };
    std::string name()
    { return _peer.GetIPPort()+" -> "+_local.GetIPPort();}
    const Buffer& inputBuffer() const { return _input; }
    const Buffer& outputBuffer() const { return _output; }
private:
    void shutdownInLoop();
    void forceCloseInLoop();


    void sendInLoop(const char* data,size_t len);

    

    void Write();   //
    void Read();
    void Close();
    void Error();
private:
    EventLoop* _loop;
    const int _sockfd;
    Channel _channel;

    std::atomic_int _state;     //TcpConnectoon 状态位
    IPAddress _local,_peer;   //本机、对等方ip

    Buffer _input;
    Buffer _output;
    size_t _highWaterMark;      //采用高水平触发

    CloseCallback _closecb;
    MessageCallback _msgcb;
    WriteCompleteCallback _writecompletecb;
    HighWaterMarkCallback _highwatermarkcallback;
};

}


#endif