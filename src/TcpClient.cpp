#include "../include/Logger.h"
#include "../include/EventLoop.h"
#include "../include/TcpConnection.h"
#include "../include/TcpClient.h"

using namespace net;



TcpClient::TcpClient(EventLoop* loop,const IPAddress& peer)
        :_loop(loop),
        _connected(false),
        _peer(peer),
        _trytimer(nullptr),
        _connector(new Connector(loop,peer)),
        _connectioncb(defaultConnectionCallback),
        _msgcb(defaultMessageCallback)
{
        _connector->setOnConnectCallback(
                std::bind(&TcpClient::OnConnection,this,_1,_2,_3));
				
}

TcpClient::~TcpClient()
{
	if(_connection && !_connection->disconnected())
		_connection->forceClose();
	if(_trytimer!=nullptr)
		_loop->cancelTimer(_trytimer);
}

//client开始运行
void TcpClient::start()
{
	_loop->assertInLoopThread();
	_connector->connect();					//connector调用sys::connect连接
	if(!_connected) _trytimer = _loop->runEvery(3s,[this](){retry();});
}

//timetask 3s 尝试一次建立连接
void TcpClient::retry()
{
	_loop->assertInLoopThread();
	if(_connected)//如果连接已经完成了，直接返回
		return;

	WARN("TcpClient::retry reconnect %s..",_peer.GetIPPort().c_str());
	_connector = std::make_unique<Connector>(_loop,_peer);	//连接完成，创建_connector
	_connector->setOnConnectCallback(std::bind(
		&TcpClient::OnConnection,this,_1,_2,_3
	));//连接时回调
	_connector->connect();
}


//连接建立时回调 sockfd+本机+目标
void TcpClient::OnConnection(int connfd,const IPAddress& local,const IPAddress&peer)
{
	//连接已经完成
	_loop->assertInLoopThread();
	if(_trytimer!=nullptr) _loop->cancelTimer(_trytimer);
	_trytimer = nullptr;
	_connected = true;
	auto connection = std::make_shared<TcpConnection>(_loop,connfd,local,peer);
	_connection = connection;

	_connection->setMessageCallback(_msgcb);
	_connection->setWriteCompleteCallback(_writeCompletecb);
	_connection->setCloseCallback(std::bind(
		&TcpClient::closeConnection,this,_1
	));
	//启用并连接channel
	_connection->connectBuildOver();	//启用channel
	_connectioncb(connection);			//打印连接信息
}


//关闭连接时回调，释放connection
void TcpClient::closeConnection(const TcpConnectionPtr& conn)
{
	_loop->assertInLoopThread();
	assert(_connection!=nullptr);
	_connection.reset();	//释放_connection
	_connectioncb(conn);	//打印连接信息
}








