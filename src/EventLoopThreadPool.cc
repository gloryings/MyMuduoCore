#include <memory>

#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"
#include "Logger.h"
/*
baseLoop_	baseLoop	保存主线程中的事件循环
name_	nameArg	保存线程池的名称
started_	false	标识线程池未启动
numThreads_	0	初始线程数为 0
next_	0	初始轮询索引为 0
hash_	ConsistentHash(3)	初始化一致性哈希，虚拟节点个数为 3
*/
EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, const std::string &nameArg)
    : baseLoop_(baseLoop), name_(nameArg), started_(false), numThreads_(0), next_(0), hash_(3)
{
}

EventLoopThreadPool::~EventLoopThreadPool()
{
    // Don't delete loop, it's stack variable
}

void EventLoopThreadPool::start(const ThreadInitCallback &cb)
{
    started_ = true;

    for (int i = 0; i < numThreads_; ++i)
    {
        char buf[name_.size() + 32];
        snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
        EventLoopThread *t = new EventLoopThread(cb, buf);
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        loops_.push_back(t->startLoop()); // 底层创建线程 绑定一个新的EventLoop 并返回该loop的地址
        hash_.addNode(buf, loops_.size() - 1);            // 将线程添加到一致哈希中。buf->size_t1 ... nums
    }

    if (numThreads_ == 0 && cb) // 整个服务端只有一个线程运行baseLoop
    {
        cb(baseLoop_);
    }
}

// 如果工作在多线程中，baseLoop_(mainLoop)会默认以轮询的方式分配Channel给subLoop
/*
线程池通过一致性哈希 (ConsistentHash) 将任务分配给对应的 EventLoop，以便在多线程环境下保持高效、稳定的事件处理能力。
使用一致性哈希算法计算 key 的哈希值
在哈希环中找到第一个大于等于该哈希值的节点
返回对应的节点在loop_中的索引
*/
EventLoop *EventLoopThreadPool::getNextLoop(const std::string &key)
{
    size_t index = hash_.getNode(key); // 获取索引
	LOG_INFO("EventLoopThreadIndex:%zu", index);	//打印选择的subeventloop id
    if (index >= loops_.size())
    {
        // 处理错误，例如返回 baseLoop 或抛出异常
        LOG_ERROR("EventLoopThreadPool::getNextLoop ERROR");
        return baseLoop_; // 或者返回 nullptr
    }
    return loops_[index]; // 使用索引访问 loops_
}


std::vector<EventLoop *> EventLoopThreadPool::getAllLoops()
{
    if (loops_.empty())
    {
        return std::vector<EventLoop *>(1, baseLoop_);
    }
    else
    {
        return loops_;
    }
}