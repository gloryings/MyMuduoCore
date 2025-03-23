#include "CurrentThread.h"

namespace CurrentThread{
	__thread int t_cachedTid = 0;
	
	void cacheTid(){
		if(t_cachedTid == 0){
			//调用系统调用获取当前tid并缓存，用户态->内核态
			t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
		}
	}
}