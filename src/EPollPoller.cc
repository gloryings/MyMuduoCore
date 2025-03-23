#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "EPollPoller.h"
#include "Logger.h"
#include "Channel.h"

const int kNew = -1;    // 某个channel还没添加至Poller          // channel的成员index_初始化为-1
const int kAdded = 1;   // 某个channel已经添加至Poller
const int kDeleted = 2; // 某个channel已经从Poller删除

//使用epoll_create1系统调用创建epoll实例，失败通过errno打印错误
EPollPoller::EPollPoller(EventLoop *loop)
	: Poller(loop)
	, epollfd_(::epoll_create1(EPOLL_CLOEXEC))
	, events_(kInitEventListSize) // vector<epoll_event>(16)
{
	if(epollfd_<0)
		LOG_FATAL("epoll_create error:%d \n", errno);
}
EPollPoller::~EPollPoller()
{
    ::close(epollfd_);	//关闭epoll实例的文件描述符
}

/*
调用 epoll_wait()，获取触发的事件数量。
如果有事件发生：
	记录调试日志
	将事件填充到 activeChannels 中
	如果事件数量达到当前 vector 的上限，进行扩容
如果超时（没有事件发生）：
	记录超时日志
如果发生错误：
	如果不是被信号中断，记录错误并设置错误码
*/
Timestamp EPollPoller::poll(int timeoutMs, ChannelList *activeChannels)	//using ChannelList = std::vector<Channel *>;
{
    // 由于频繁调用poll 实际上应该用LOG_DEBUG输出日志更为合理 当遇到并发场景 关闭DEBUG日志提升效率
    LOG_DEBUG("func=%s => fd total count:%lu\n", __FUNCTION__, channels_.size());

	//获取epoll监听到的活跃事件数，并把发生事件的文件描述符存入events_
    int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);
    int saveErrno = errno;
    Timestamp now(Timestamp::now());

    if (numEvents > 0)
    {
        LOG_DEBUG("%d events happend\n", numEvents); // LOG_DEBUG最合理
        fillActiveChannels(numEvents, activeChannels);	//将events_中的信息加入activeChannels
        if (numEvents == events_.size()) // 说明下次可能存在更多活跃事件，扩容操作，减少下次自动扩容开销
        {
            events_.resize(events_.size() * 2);
        }
    }
    else if (numEvents == 0)
    {
        LOG_DEBUG("%s timeout!\n", __FUNCTION__);	//EPollPoller::poll() timeout!
    }
    else
    {
        if (saveErrno != EINTR)
        {
            errno = saveErrno;
            LOG_ERROR("EPollPoller::poll() error!");
        }
    }
    return now;
}

// channel update remove => EventLoop updateChannel removeChannel => Poller updateChannel removeChannel
/*
🌟 整体作用
Channel：表示一个文件描述符（fd）及其相关的事件（如可读、可写等）。

EPollPoller::updateChannel()：根据 Channel 的状态（新建、删除或修改）在 epoll 中执行对应的操作。

三种操作模式：
	EPOLL_CTL_ADD → 注册新的事件
	EPOLL_CTL_MOD → 修改已存在的事件
	EPOLL_CTL_DEL → 删除已注册的事件

*/
void EPollPoller::updateChannel(Channel *channel)
{
    const int index = channel->index();	//获取channel是否已加入epoll实例中
    LOG_INFO("func=%s => fd=%d events=%d index=%d\n", __FUNCTION__, channel->fd(), channel->events(), index);	//func=updateChannel => fd=3 events=5 index=-1

    if (index == kNew || index == kDeleted)
    {
        if (index == kNew)	//将 channel 注册到 channels_ (map<int, Channel*>)
        {
            int fd = channel->fd();
            channels_[fd] = channel;	
        }
        else // index == kDeleted
        {
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);	//EPOLL_CTL_ADD 1
    }
    else // channel已经在Poller中注册过了
    {
        int fd = channel->fd();
        if (channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else
        {
            update(EPOLL_CTL_MOD, channel);	//EPOLL_CTL_MOD 3
        }
    }
}

// 从Poller中删除channel
void EPollPoller::removeChannel(Channel *channel)
{
    int fd = channel->fd();
    channels_.erase(fd);

    LOG_INFO("func=%s => fd=%d\n", __FUNCTION__, fd);

    int index = channel->index();
    if (index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);	//EPOLL_CTL_DEL 2
    }
    channel->set_index(kNew);
}

// 填写活跃的连接
void EPollPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const
{
    for (int i = 0; i < numEvents; ++i)
    {
        Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel); // EventLoop就拿到了它的Poller给它返回的所有发生事件的channel列表了
    }
}

// 更新channel通道 其实就是调用epoll_ctl add/mod/del
void EPollPoller::update(int operation, Channel *channel)
{
    epoll_event event;
    ::memset(&event, 0, sizeof(event));

    int fd = channel->fd();

    event.events = channel->events();
    event.data.fd = fd;
    event.data.ptr = channel;

    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0)
    {
        if (operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR("epoll_ctl del error:%d\n", errno);
        }
        else
        {
            LOG_FATAL("epoll_ctl add/mod error:%d\n", errno);
        }
    }
}