#pragma once

#include "noncopyable.h"

class InetAddress;	//前置声明InetAddress类，减少头文件之间的依赖

//封装socket fd
class Socket : noncopyable{
	public:
		//explicit防止隐式转换，避免将int识别成socket
		explicit Socket(int sockfd)
			: sockfd_(sockfd)
		{
		}
		~Socket();

		int fd() const { return sockfd_; }
		void bindAddress(const InetAddress &localaddr);	//将socket绑定到本地的地址(ip:port)上
		void listen();	//将socket置于监听状态
		int accept(InetAddress *peeraddr);

		void shutdownWrite();

		void setTcpNoDelay(bool on);	//设置TCP_NODELAY选项，控制Nagle算法，on = true 表示禁用 Nagle 算法，直接发送小包，降低延迟
		void setReuseAddr(bool on);
		void setReusePort(bool on);
		void setKeepAlive(bool on);

	private:
		const int sockfd_;
};