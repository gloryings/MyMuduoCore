#pragma once

#include <functional>
#include <thread>
#include <memory>
#include <unistd.h>
#include <string>
#include <atomic>

#include "noncopyable.h"

class Thread : noncopyable
{
public:
    using ThreadFunc = std::function<void()>;

    explicit Thread(ThreadFunc, const std::string &name = std::string());
    ~Thread();

    void start();	// 启动线程
    void join();	// 等待线程结束

    bool started() { return started_; }	 // 检查线程是否已经启动
    pid_t tid() const { return tid_; }	 // 获取线程ID
    const std::string &name() const { return name_; }	// 获取线程名称

    static int numCreated() { return numCreated_; }		// 获取已创建线程的数量

private:
    void setDefaultName();

    bool started_;
    bool joined_;
    std::shared_ptr<std::thread> thread_;	// 使用智能指针管理线程
    pid_t tid_;       // 在线程创建时再绑定
    ThreadFunc func_; // 线程回调函数
    std::string name_;
    static std::atomic_int numCreated_;	//原子变量保证多线程环境下对该变量的互斥操作
};