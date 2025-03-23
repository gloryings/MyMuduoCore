#include <sys/epoll.h>
#include "Channel.h"
#include "EventLoop.h"
#include "Logger.h"

const int Channel::kNoneEvent = 0; //空事件
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI; //读事件，EPOLLIN → 表示有数据可读（比如 socket 收到数据） EPOLLPRI → 表示有紧急数据到来（如带外数据，Out-of-Band Data）
const int Channel::kWriteEvent = EPOLLOUT; //写事件，表示当前 socket 已经可写（发送缓冲区已空，可以继续发送数据）

// 构造函数
Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop)
    , fd_(fd)
    , events_(0)
    , revents_(0)
    , index_(-1)
    , tied_(false)
{
}

Channel::~Channel()
{
}

// channel的tie方法什么时候调用过?  TcpConnection => channel
/**
 * TcpConnection中注册了Channel对应的回调函数，传入的回调函数均为TcpConnection
 * 对象的成员方法，因此可以说明一点就是：Channel的结束一定晚于TcpConnection对象！
 * 此处用tie去解决TcpConnection和Channel的生命周期时长问题，从而保证了Channel对象能够在
 * TcpConnection销毁前销毁。
 * 使用 std::weak_ptr 防止循环引用，确保 TcpConnection 生命周期由 Channel 持有,可通过将其转换成shared判断TcpConnection是否被释放
 **/
void Channel::tie(const std::shared_ptr<void> &obj)
{
	tie_ = obj;
	tied_ = true;
}

//update 和remove => EpollPoller 更新channel在poller中的状态
/**
 * 当改变channel所表示的fd的events事件后，update负责再poller里面更改fd相应的事件epoll_ctl
 **/
void Channel::update()
{
	// 通过channel所属的eventloop，调用poller的相应方法，注册fd的events事件，updateChannel() 会调用 epoll_ctl() 更新 epoll 的监听事件。
	loop_->updateChannel(this);
}

// 在channel所属的EventLoop中把当前的channel删除掉
void Channel::remove()
{
    loop_->removeChannel(this);
}


// 处理事件，防止TcpConnection提前销毁，导致悬空指针
void Channel::handleEvent(Timestamp receiveTime)
{
    if (tied_)
    {
		//weak_ptr → 避免循环引用
		// tie_.lock() → 尝试将 weak_ptr 转换成 shared_ptr
		/*如果 tie_ 所指向的对象存在且未被销毁：
		lock() 返回一个有效的 shared_ptr，增加引用计数
		如果 tie_ 所指向的对象已经被销毁：
		lock() 返回一个空的 shared_ptr
		shared_ptr 保证TcpConnection在 Channel 使用期间不会被析构
		*/
        std::shared_ptr<void> guard = tie_.lock();	
        if (guard)
        {
            handleEventWithGuard(receiveTime);
        }
        // 如果提升失败了 就不做任何处理 说明Channel的TcpConnection对象已经不存在了
    }
    else
    {
        handleEventWithGuard(receiveTime);
    }
}

// 具体事件处理
void Channel::handleEventWithGuard(Timestamp receiveTime)
{
    LOG_INFO("channel handleEvent revents:%d\n", revents_);	//打印poller监听到具体事件
    // 关闭
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) // 当TcpConnection对应Channel 通过shutdown 关闭写端 epoll触发EPOLLHUP
    {
        if (closeCallback_)
        {
            closeCallback_();
        }
    }
    // 错误
    if (revents_ & EPOLLERR)
    {
        if (errorCallback_)
        {
            errorCallback_();
        }
    }
    // 读
    if (revents_ & (EPOLLIN | EPOLLPRI))
    {
        if (readCallback_)
        {
            readCallback_(receiveTime);
        }
    }
    // 写
    if (revents_ & EPOLLOUT)
    {
        if (writeCallback_)
        {
            writeCallback_();
        }
    }
}