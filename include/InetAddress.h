#pragma once

#include<arpa/inet.h>
#include<netinet/in.h>
#include<string>

//封装socket的地址类型和端口，封装和管理网络地址和端口，提供传输层通信所需要的地址信息
class InetAddress{
	public:
		//通过ip和port构造
		explicit InetAddress(uint16_t port=0, std::string ip="127.0.0.1");
		explicit InetAddress(const sockaddr_in& addr) : addr_(addr){}

		std::string toIp() const;		//代表该函数不会改变成员变量,即this指向的是常量
		std::string toIpPort() const;
		uint16_t toPort() const;

		const sockaddr_in* getSockAddr() const {return &addr_;}		//第一个const代表这个函数返回的是常量指针，无法通过该指针改变对象数据，第二个const代表该函数不会改变成员变量,即this指向的是常量
		void setSockAddr(const sockaddr_in &addr){addr_ = addr;}	//设置socket的IP和端口




	private:
		sockaddr_in addr_;
};