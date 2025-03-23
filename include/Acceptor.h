#pragma once

#include <functional>

#include "noncopyable.h"
#include "Socket.h"
#include "Channel.h"

class EventLoop;
class InetAddress;
/*
Acceptor 用于在 mainLoop 中接收新连接，通过回调通知 TcpServer 生成 TcpConnection
通过 acceptSocket_ 监听新的连接请求。
当有新连接到来时，触发 handleRead() 进行处理。
在处理新连接时，调用 NewConnectionCallback_ 回调，通知上层逻辑（通常是创建新的 TcpConnection）。
TcpServer::start() → Acceptor::listen() → handleRead() ->NewConnectionCallback_() → TcpServer::newConnection()
*/
class Acceptor : noncopyable
{
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress &)>;

    Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport);
    ~Acceptor();
    //设置处理了新连接的回调函数
    void setNewConnectionCallback(const NewConnectionCallback &cb) { NewConnectionCallback_ = cb; }
    // 判断是否在监听
    bool listenning() const { return listenning_; }
    // 监听本地端口
    void listen();

private:
    void handleRead();//处理新用户的连接事件

    EventLoop *loop_; // Acceptor用的就是用户定义的那个baseLoop 也称作mainLoop
    Socket acceptSocket_;//专门用于接收新连接的socket
    Channel acceptChannel_;//专门用于监听新连接的channel
    NewConnectionCallback NewConnectionCallback_;//新连接的回调函数
    bool listenning_;//是否在监听
};