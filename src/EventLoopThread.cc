#include "EventLoopThread.h"
#include "EventLoop.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb,
								 const std::string &name)
	: loop_(nullptr)
	, exiting_(false)
	, thread_(std::bind(&EventLoopThread::threadFunc,this), name)	//通过 std::bind 绑定 threadFunc 到 thread_，当调用 thread_.start() 时会在线程中执行 threadFunc()。
	, mutex_()
	, cond_()
	, callback_(cb)
{

}

EventLoopThread::~EventLoopThread()
{
	exiting_ = true;
	if(loop_ != nullptr){
		loop_->quit();
		thread_.join();
	}
}

EventLoop *EventLoopThread::startLoop()
{
    thread_.start(); // 启用底层线程Thread类对象thread_中通过start()创建的线程，会调用EventLoopThread::threadFunc()

    EventLoop *loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);	 // 加锁，保护共享资源loop_访问
        cond_.wait(lock, [this](){return loop_ != nullptr;});	// 等待 loop_ 被设置
        loop = loop_;
    }
    return loop;
}

// 下面这个方法 是在单独的新线程里运行的
void EventLoopThread::threadFunc()
{
    EventLoop loop; // 创建一个独立的EventLoop对象 和上面的线程是一一对应的 级one loop per thread

    if (callback_)
    {
        callback_(&loop);
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();	//通知等待的主线程。
    }
    loop.loop();    // 开启事件循环，执行EventLoop的loop() 开启了底层的Poller的poll()

	/*loop.loop() 执行结束。
	EventLoop::quit() 被调用，导致事件循环退出。*/
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}