#include <cassert>
#include <unistd.h>
#include <sys/epoll.h>
#include "../include/Logger.h"
#include "../include/EventLoop.h"
#include "../include/TcpConnection.h"

using namespace net;


namespace net
{
enum ConnectionState
{
    Connecting=0,
    Connected,
    Disconnecting,
    Disconnected
};

//缺省线程初始化回调
void defaultThreadInitCallback(size_t index)
{
    TRACE("EventLoop thread [id: %lu] stated",index);
}

//缺省连接建立时回调
void defaultConnectionCallback(const TcpConnectionPtr& conn)
{
    INFO("connection %s -> %s %s ",
        conn->peer().GetIPPort().c_str(),
        conn->local().GetIPPort().c_str(),
        conn->connected() ? "up":"down");
}

//缺省
void defaultMessageCallback(const TcpConnectionPtr& conn,Buffer& buff)
{
    TRACE("connection %s -> %s recv %lu bytes",
          conn->peer().GetIPPort().c_str(),
          conn->local().GetIPPort().c_str(),
          buff.ReadableBytes());
    buff.InitAll(); //初始化缓冲区
}
}


TcpConnection::TcpConnection(EventLoop* loop,int sockfd,
            const IPAddress& local,
            const IPAddress& peer)
        :_loop(loop),
        _sockfd(sockfd),
        _channel(_loop,_sockfd),
        _state(Connecting),
        _local(local),
        _peer(peer),
        _highWaterMark(0)
{
    _channel.setReadCallback([this](){Read();});
    _channel.setCloseCallback([this](){Close();});
    _channel.setErrorCallbak([this](){Error();});
    _channel.setWriteCallback([this](){Write();});

    TRACE("TcpConnection() %s  fd=%d",name().c_str(),_sockfd);
}

TcpConnection::~TcpConnection()
{
    assert(_state == Disconnected); //每断开连接就析构，则失败
    ::close(_sockfd);               //关闭套接字

    TRACE("~TcpConnection() %s fd=%d", name().c_str(), _sockfd);
}



//连接建立完成
void TcpConnection::connectBuildOver()
{
    assert(_state == Connecting);
    _state = Connected;
    _channel.tie(shared_from_this());
    _channel.enableRead();  //注册可读事件
}



//发送数据
void TcpConnection::send(const char* data,size_t len)
{
    if(_state != Connected){
        WARN("TcpConnection::send() connected,can't send!");
        return;
    }
    //当前线程，直接发送
    if(_loop->isInLoopThread()){
        sendInLoop(data,len);   
    }
    else
    {
        _loop->addTask(
            [ptr = shared_from_this(),str = std::string(data,data+len)]()
            {ptr->sendInLoop(str.c_str(),sizeof(str.c_str()));}
        );
    }
}


//发送数据
void TcpConnection::sendInLoop(const char* data,size_t len)
{
    _loop->assertInLoopThread();

    if(_state == Disconnected){//连接以断开
        WARN("TcpConnection::sendInLoop() disconnected, can't send!");
        return;
    }
    int n = 0;          //已写字节数
    size_t re = len;    //剩余字节数
    bool error = false;

    if(!_channel.isWriting())//如果没有正在写入
    {
        assert(_output.ReadableBytes() == 0);   //确保缓冲区空
        n = ::write(_sockfd,data,len);          //写了n字节
        //错误了
        if(n=-1){
            if(errno != EAGAIN){ //非阻塞，可能调用失败
                ERROR("TcpConnection::write()");
                if(errno == EPIPE)  //close_wait状态发送数据会触发epipe
                    error = true;
                if(errno == ECONNRESET) //
                    error = true;
            }
            n=0;
        }
        else//没有错误
        {
            re -= static_cast<size_t>(n);   //更新剩余字节数
            if(re == 0 && _writecompletecb)
            {//writecb 加入eventloop
                _loop->addTask(std::bind(
                    _writecompletecb,shared_from_this()));
            }
        }
    }


    //如果没有出现错误，且还有剩余字节无法写入buffer中
    if(!error && re>0){
        //将剩余数据写入 _output
        if(_highwatermarkcallback)
        {
            size_t oldlen = _output.ReadableBytes();//当前可读字节数
            size_t newlen = oldlen+re;              //剩余字节数
            if(oldlen < _highWaterMark && newlen >= _highWaterMark)
            {
                _loop->addTask(std::bind(
                    _highwatermarkcallback,shared_from_this(),newlen
                ));
            }
            _output.WriteString(data+n,re);
            _channel.enableWrite();
        }
    }
}


//showdown优雅关闭，套接字资源没有释放，相当于刷新缓存
void TcpConnection::shutdown()
{
    bool isDisConnecting=false;
    assert(_state != Disconnected);
    
    if(_state==Connected)
        isDisConnecting = true;
    _state = Disconnecting;
    
    if(isDisConnecting) //正在断开连接
    {
        if(_loop->isInLoopThread())
        {
            shutdownInLoop();
        }
        else
            _loop->addTask(std::bind(&TcpConnection::shutdownInLoop,shared_from_this()));
    }
}

void TcpConnection::shutdownInLoop()
{
        _loop->assertInLoopThread();
        if(_state != Disconnected && !_channel.isWriting()){
            if(-1 == ::shutdown(_sockfd,SHUT_WR))
                ERROR("TcpConnection::shutdown()");
        }
}

//强制关闭外部接口
void TcpConnection::forceClose()
{
    bool isDisConnecting=false;
    assert(_state != Disconnected);
    if(_state==Connected)
        isDisConnecting = true;
    _state = Disconnecting;

    if(isDisConnecting)
    {
        if(_loop->isInLoopThread())
            forceCloseInLoop();
        else
            _loop->addTask(std::bind(&TcpConnection::forceCloseInLoop,shared_from_this()));
    }
}

//强制关闭
void TcpConnection::forceCloseInLoop()
{
    _loop->assertInLoopThread();
    if(_state != Disconnected)
        Close();
}


//sockfd读取数据
void TcpConnection::Read()
{
    _loop->assertInLoopThread();
    assert(_state != Disconnected);
    int Errnocode;
    int64_t n = _input.readfd(_sockfd,Errnocode);   //从sockfd中接收数据
    if(n==-1){
        errno = Errnocode;
        ERROR("TcpConnection::Read() error!");
        Error();
    }
    else if (n == 0)    //获取数据量为0，调用close
        Close();
    else
        _msgcb(shared_from_this(), _input);
    
}

//sockfd发送数据
void TcpConnection::Write()
{
    if (_state == Disconnected) {   //如果已经断开连接，肯定出错
        WARN("TcpConnection::handleWrite() disconnected, "
                     "give up writing %lu bytes", _output.ReadableBytes());
        return;
    }
    assert(_output.ReadableBytes() > 0);    //当前可读字节大于0
    assert(_channel.isWriting());  //且是可写事件
    ssize_t n = ::write(_sockfd, _output.peek(), _output.ReadableBytes());
    if (n == -1) {
        ERROR("TcpConnection::write()");
    }
    else {
        _output.recycle(static_cast<size_t>(n));   //回收output中空间，前面读走多少，我们改变多少
        if (_output.ReadableBytes() == 0) {
            _channel.disableWrite();
            if (_state == Disconnecting)
                shutdownInLoop();
            if (_writecompletecb) {
                _loop->addTask(std::bind(
                        _writecompletecb, shared_from_this()));
            }
        }
    }
}

//tcpconnection关闭，需要将文件描述符从epoller和eventloop中撤销，并调用回调通知
void TcpConnection::Close()
{
    _loop->assertInLoopThread();
    assert(_state == Connected ||
           _state == Disconnecting);
    _state = Disconnected;
    _loop->removeChannel(&_channel);
    _closecb(shared_from_this());
}


void TcpConnection::Error()
{
    int err;
    socklen_t len = sizeof(err);
    int ret = getsockopt(_sockfd, SOL_SOCKET, SO_ERROR, &err, &len);    //获取错误信息
    if (ret != -1)
        errno = err;
    ERROR("TcpConnection::handleError() err_code:%d",err);
}

//如果fd可读设置为不可读
void TcpConnection::stopRead()
{
    _loop->addTask([this](){
        if(_channel.isReading())
            _channel.disableRead(); //设置不可读
    });
}

//如果fd不可读设置为可读
void TcpConnection::startRead()
{
    _loop->addTask([this](){
        if(!_channel.isReading())
            _channel.enableRead();  //设置可读
    });
}

bool TcpConnection::connected() const
{ return _state == Connected;}

bool TcpConnection::disconnected() const
{ return _state == Disconnected;}
