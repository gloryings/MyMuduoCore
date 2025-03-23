#include <strings.h>
#include <string.h>

#include "InetAddress.h"

InetAddress::InetAddress(uint16_t port, std::string ip){
	::memset(&addr_, 0, sizeof(addr_));					//特指使用全局作用域中的memeset，
	addr_.sin_family = AF_INET;							//设置地址族为 AF_INET，表示使用 IPv4 协议。
    addr_.sin_port = ::htons(port); 					// 将端口号从 主机字节序（主机存储顺序）转换为 网络字节序（大端存储）。
    addr_.sin_addr.s_addr = ::inet_addr(ip.c_str());	//将 ip 字符串（如 "127.0.0.1") 转换为网络字节序的整数
}

std::string InetAddress::toIp() const{
    // ip
    char buf[64] = {0};
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);	//将 sockaddr_in 结构体中的 IP 地址转换为字符串格式（如 "127.0.0.1"），存储在buf中。
    return buf;
}

std::string InetAddress::toIpPort() const{
    // ip:port
    char buf[64] = {0};
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
    size_t end = ::strlen(buf);
    uint16_t port = ::ntohs(addr_.sin_port);	//将端口号从 网络字节序 转换为 主机字节序。
    sprintf(buf+end, ":%u", port);	//在ip末尾加上:port
    return buf;
}

uint16_t InetAddress::toPort() const{
    return ::ntohs(addr_.sin_port);
}


// 测试程序
// #include <iostream>
// int main(){
//     InetAddress addr(8080);
//     std::cout << addr.toIpPort() << std::endl;
// }

