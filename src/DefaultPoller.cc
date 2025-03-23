#include <stdlib.h>

#include "Poller.h"
#include "EPollPoller.h"
// 工厂类（封装创建逻辑）
//工厂模式封装了对象的创建逻辑，返回一个基类指针或引用（依赖多态）。
//工厂方法返回的对象通过多态机制，动态绑定到具体的子类实现。
/*
多态性的核心是通过 基类指针或引用 调用派生类的重写方法。
调用 poller->updateChannel() 时，由于基类定义为 virtual 函数，调用会被动态绑定到 EPollPoller::updateChannel() 的实现。
📌 在底层，动态绑定是如何实现的？
每个带有虚函数的类都有一个 虚函数表（vtable）。
编译器在生成对象时，给对象绑定一个指向 vtable 的指针（vptr）。
当通过基类指针调用虚函数时，程序会通过 vptr 查找 vtable 中对应的函数地址，并在运行时调用该函数。
地址		内容					说明
0x1000	vptr -> 0x2000		指向 Derived 的 vtable
0x1008	成员变量1				对象的实际成员变量
0x1010	成员变量2				对象的实际成员变量
0x2000	Derived::func1()	vtable[0]，覆盖自 Base
0x2008	Derived::func2()	vtable[1]，覆盖自 Base
*/

Poller *Poller::newDefaultPoller(EventLoop *loop){
	if(::getenv("MUDUO_USE_POLL")){	//根据环境变量选择对应的poll类型
		return nullptr;	//暂不支持poll类型监听
	}
	else{
		return new EPollPoller(loop);	//生成epoll实例
	}
}