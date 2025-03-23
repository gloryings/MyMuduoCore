#pragma once

//统一封装函数、Lambda 等可调用对象，提供类型安全的回调机制
//回调函数是一个通过函数指针或可调用对象传递的函数，在特定事件（如异步操作、事件触发）发生时被调用。它允许将代码逻辑作为参数传递，实现解耦和灵活性。
#include <functional>	
/*
std::unique_ptr
独占所有权，不可复制，可通过 std::move 转移。
替代 new/delete，适用于单一所有者场景。

std::shared_ptr
共享所有权，通过引用计数管理资源。
多个指针共享同一对象，计数归零时释放内存。

std::weak_ptr
观察 shared_ptr 但不增加引用计数。
解决循环引用问题（如 A→B→A 的循环）。
*/

#include <memory>		//智能指针库

#include "noncopyable.h"
#include "Timestamp.h"

class EventLoop;

/**
 * 理清楚 EventLoop、Channel、Poller之间的关系  Reactor模型上对应多路事件分发器
 * Channel理解为通道 封装了sockfd和其感兴趣的event 如EPOLLIN、EPOLLOUT事件 还绑定了poller返回的具体事件
 **/

class Channel : noncopyable{
public:
	/*setCallback() → 预设好触发事件时执行的操作
	click() → 触发事件
	if (callback) → 确认有设置回调
	callback() → 执行你设置的操作*/

	/*
	std::function 提供了一种统一的方式来处理各种可调用对象。
	当你需要传递回调函数、存储多个不同类型的可调用对象或者在运行时动态决定要调用哪个函数时，std::function 会非常有用。
	它的优势在于你不需要知道具体的函数类型或参数类型，它会处理各种类型的可调用对象。
	*/
	using EventCallback = std::function<void()>;	//用using代替typedef
	using ReadEventCallback = std::function<void(Timestamp)>;

	Channel(EventLoop *loop, int fd);
	~Channel();

	// fd得到Poller通知以后，处理事件handleEvent在EventLoop::loop()中调用，根据事件类型调用对应的回调
	void handleEvent(Timestamp receiveTime);

	// 设置回调函数对象
	//外部channel.setReadCallback(bind())，若readCallback_是public，等价于外部channel.readCallback_ = bind()
	void setReadCallback(ReadEventCallback cb){
		readCallback_ = std::move(cb);	//move可以直接传递，减少拷贝
	}
	void setWriteCallback(EventCallback cb){
		writeCallback_ = std::move(cb);
	}
	void setCloseCallback(EventCallback cb){
		closeCallback_ = std::move(cb);
	}
	void setErrorCallback(EventCallback cb){
		errorCallback_ = std::move(cb);
	}

	// 防止当Tcpconnection被手动remove掉，channel还在执行回调操作
	void tie(const std::shared_ptr<void> &);

	//获取文件描述符和感兴趣事件
	int fd() const {return fd_;}
	int events() const {return events_;}
    void set_revents(int revt) { revents_ = revt; }	//poll监听到发生的事件

	// 设置fd相应的事件状态 相当于epoll_ctl add delete， update将修改后的事件标志传递给epoll，即注册感兴趣的事件
	void enableReading() { events_ |= kReadEvent; update(); }
	void disableReading() { events_ &= ~kReadEvent; update(); }
	void enableWriting() { events_ |= kWriteEvent; update(); }
	void disableWriting() { events_ &= ~kWriteEvent; update(); }
	void disableAll() { events_ = kNoneEvent; update(); }

    // 返回fd当前的事件状态
    bool isNoneEvent() const { return events_ == kNoneEvent; }
    bool isWriting() const { return events_ & kWriteEvent; }
    bool isReading() const { return events_ & kReadEvent; }

	/*
	const int kNew = -1;    // 某个channel还没添加至Poller          // channel的成员index_初始化为-1
	const int kAdded = 1;   // 某个channel已经添加至Poller
	const int kDeleted = 2; // 某个channel已经从Poller删除
	用于update时判断channel状态是add还是modify
	*/
    int index() { return index_; }				
    void set_index(int idx) { index_ = idx; }	

	//ownerLoop() → 返回 Channel 所属的 EventLoop
	//remove() → 从 EventLoop 中移除
	EventLoop *ownerLoop() { return loop_; }
	void remove();
private:

	void update();	//通知 Poller 更新监听状态
	void handleEventWithGuard(Timestamp receiveTime);	//事件处理封装,调用触发事件对应设置的回调

	//静态常量定义
	static const int kNoneEvent;	//未监听任何事件
	static const int kReadEvent;	//监听读事件
	static const int kWriteEvent;	//监听写事件

	EventLoop *loop_; // 所属的事件循环，subreactor
	const int fd_;    // fd，Poller监听的对象
	int events_;      // 注册fd感兴趣的事件，需要监听的事件
	int revents_;     // Poller返回的具体发生的事件，实际发生的事件
	int index_;

	std::weak_ptr<void> tie_;	//防止tcpconnection在channel之前销毁，此时event可能是绑定的tcpconnection的成员函数，会访问到空指针
	bool tied_;		//标记是否绑定对象	

	//定义回调FUNCTION，可存储函数、lamda表达式等可调用对象
	ReadEventCallback readCallback_;
	EventCallback writeCallback_;
	EventCallback closeCallback_;
	EventCallback errorCallback_;


};