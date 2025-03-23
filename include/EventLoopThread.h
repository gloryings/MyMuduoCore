#pragma once

#include <functional>
#include <mutex>
#include <condition_variable>	//用于线程间的通知机制。
#include <string>

#include "noncopyable.h"
#include "Thread.h"

class EventLoop;
/*
EventLoopThread的核心功能是：用于主eventloop给subeventloop分发事件
	创建一个新的线程。
	在新线程中启动一个 EventLoop。
	支持线程安全的启动和同步。
*/
class EventLoopThread : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;	//线程初始化回调函数类型，传入一个EventLoop指针

    EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(),
                    const std::string &name = std::string());
    ~EventLoopThread();

    EventLoop *startLoop();

private:
    void threadFunc();	//线程函数，用于在线程中执行事件循环。

    EventLoop *loop_;
    bool exiting_;	//用于标识当前对象是否退出。
    Thread thread_;
    std::mutex mutex_;             // 互斥锁,保护对共享资源的访问
    std::condition_variable cond_; // 条件变量,用于在线程间实现同步和通知。
    ThreadInitCallback callback_;
};