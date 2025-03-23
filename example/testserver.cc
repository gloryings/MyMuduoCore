#include <string>

#include "TcpServer.h"
#include "Logger.h"
/*
1.客户端连接：
	客户端通过 socket 连接服务器。
2.数据接收：
	客户端发送数据，服务器接收。
3.数据回送：
	服务器将接收的数据直接返回给客户端。
4.关闭连接（可选）：
	短连接场景下，服务器在发送完数据后关闭连接。
	长连接场景下，服务器保持连接状态，等待新的数据
*/
class EchoServer
{
public:
	EchoServer(EventLoop *loop, const InetAddress &addr, const std::string &name)
		: server_(loop, addr, name)
		, loop_(loop)
		// 注册回调函数
		server_.setConnectionCallback(
			std::bind(&EchoServer::onConnection, this, std::placeholders::_1)
		);

		server_.setMessageCallback(
			std::bind(&EchoServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
		);

		// 设置合适的subloop线程数量
		server_.setThreadNum(3);
	}

	void start()
	{
		server_.start();
	}

private:
	// 连接建立或断开的回调函数
	void onConnection(const TcpConnectionPtr &conn){
		if(conn->connected()){
			LOG_INFO("Connection UP : %s", conn->peerAddress().toIpPort().c_str());
		}
		else{
			LOG_INFO("Connnection DOWN : %s", conn->peerAddress().toIpPort().c_str());
		}
	}

	// 可读写事件回调
	void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time){
		std::string msg = buf->retrieveAllAsString();
		conn->send(msg);
		// conn->shutdown();	// 关闭写端，底层相应EPOLLHUP => 执行closeCallback_，在短期链接时发完消息可以关闭
	}
	TcpServer server_;
	EventLoop *loop_;
};

int main(){
	EventLoop loop;
	InetAddress addr(8080);
	EchoServer server(&loop, addr, "EchoServer");
	server.start();	//负责管理tcpconnection
	loop.loop();	//负责监听和分发事件，不在server.start()里面启动是因为做到职责分离，支持一对多
	return 0;
}