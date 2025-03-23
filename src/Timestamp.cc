#include<time.h>
#include"Timestamp.h"

Timestamp::Timestamp() : microSecondsSinceEpoch_(0){}

Timestamp::Timestamp(int64_t microSecondsSinceEpoch) 
	: microSecondsSinceEpoch_(microSecondsSinceEpoch){}//time(NULL) 返回当前的系统时间（从 1970 年 1 月 1 日开始的秒数）。

Timestamp Timestamp::now(){
	return Timestamp(time(NULL));
}

std::string Timestamp::toString() const{
	char buf[128] = {0};//定义存储字符串并初始化
	tm *tm_time = localtime(&microSecondsSinceEpoch_);
	//使用 snprintf() 函数将 tm_time 中的各个字段（年、月、日、时、分、秒）格式化为字符串，并存入 buf 中。
	snprintf(buf, 128, "%4d/%02d/%02d %02d:%02d:%02d",
		tm_time->tm_year + 1900,
		tm_time->tm_mon + 1,
		tm_time->tm_mday,
		tm_time->tm_hour,
		tm_time->tm_min,
		tm_time->tm_sec);
	return buf;
}

// 测试时间戳类实现是否正确
// #include <iostream>
// int main() {
//     std::cout << Timestamp::now().toString() << std::endl;
//     return 0;
// }