#include "../include/Logger.h"
#include "../include/TcpConnection.h"
#include "../include/EventLoop.h"
#include "../include/Buffer.h"
#include "../include/TcpServerSingle.h"

using namespace net;


TcpServerSingle::TcpServerSingle(EventLoop* loop,const IPAddress& local)
        :_loop(loop),
        _accept(loop,local)
{
    _accept.SetOnConnect(
        std::bind(&TcpServerSingle::OnConnection,this,_1,_2,_3)
    );
}

//开始监听
void TcpServerSingle::start()
{
    _accept.listen();
}

//建立连接
void TcpServerSingle::OnConnection(int connfd,const IPAddress& local,const IPAddress& peer)
{
    _loop->assertInLoopThread();
    auto conn = std::make_shared<TcpConnection>(_loop,connfd,local,peer);
    _connections.insert(conn);
    conn->setMessageCallback(_msgcb);
    conn->setWriteCompleteCallback(_writeCompletecb);
    conn->setCloseCallback(std::bind(
            &TcpServerSingle::closeConnection, this, _1));
    // enable and tie channel
    conn->connectBuildOver();   //建立连接，启动channel
    _connectioncb(conn);        //打印数据
}

//连接关闭时回调
void TcpServerSingle::closeConnection(const TcpConnectionPtr& conn)
{
    _loop->assertInLoopThread();
    size_t ret = _connections.erase(conn);
    assert(ret == 1);(void)ret;
    _connectioncb(conn);
}

void TcpServerSingle::setConnectionCallback(const ConnectionCallback& cb)
{
    _connectioncb = cb;
}

void TcpServerSingle::setMessageCallback(const MessageCallback& cb)
{
    _msgcb = cb;
}
void TcpServerSingle::setWriteCompleteCallback(const WriteCompleteCallback& cb)
{
    _writeCompletecb = cb;
}
