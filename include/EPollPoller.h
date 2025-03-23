#pragma once

#include <vector>
#include <sys/epoll.h>

#include "Poller.h"
#include "Timestamp.h"

/**
 * epoll的使用:
 * 1. epoll_create
 * 2. epoll_ctl (add, mod, del)
 * 3. epoll_wait
 **/

class Channel;		// 仅指针和引用时可以使用，类的前向声明，避免循环引用，用于解决类之间相互依赖的问题

class EPollPoller : public Poller
{
public:
    EPollPoller(EventLoop *loop);
    ~EPollPoller() override;

    // 重写基类Poller的抽象方法
    Timestamp poll(int timeoutMs, ChannelList *activeChannels) override;	//using ChannelList = std::vector<Channel *>;
    void updateChannel(Channel *channel) override;
    void removeChannel(Channel *channel) override;

private:
    static const int kInitEventListSize = 16;	//初始事件列表大小

    // 填写活跃的连接
    void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;
    // 更新channel通道的感兴趣事件等 其实就是调用epoll_ctl
    void update(int operation, Channel *channel);

	/*struct epoll_event {
		uint32_t events; // 事件类型，如 EPOLLIN, EPOLLOUT, 等
		epoll_data_t data; // 用户定义的数据，常用来存储文件描述符或指针
	};*/
    using EventList = std::vector<epoll_event>; // C++中可以省略struct 直接写epoll_event即可

    int epollfd_;      // epoll_create创建返回的fd保存在epollfd_中
    EventList events_; // 用于存放epoll_wait返回的所有发生的事件的文件描述符事件集
};