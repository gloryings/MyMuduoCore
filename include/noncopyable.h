#pragma once //防止头文件重复
/**
 * noncopyable被继承后 派生类对象可正常构造和析构 但派生类对象无法进行拷贝构造和赋值构造
   对于涉及系统资源（如线程、网络连接、文件描述符等）的类，拷贝可能导致：
	资源竞争
	残留指针
	内存泄漏
	通过继承 noncopyable，可以让这些类对象在语义上明确为不可复制。
 **/
 class noncopyable{
	public:
		noncopyable(const noncopyable &) = delete;
		noncopyable &operator=(const noncopyable &) = delete;

	protected:
		noncopyable() = default;
		~noncopyable() = default;
 };