#include<iostream>

#include "Logger.h"
#include "Timestamp.h"

//获取日志的唯一实例对象，单例
Logger &Logger::instance(){
	static Logger logger;
	return logger;
}

//设置日志级别
void Logger::setLogLevel(int level){
	logLevel_ = level;
}

//写日志： [级别信息] time : msg
void Logger::log(std::string msg){
	std::string pre = "";
	//读取日志级别并输出
	switch(logLevel_){
		case INFO:
			pre = "[INFO]";
			break;
		case ERROR:
			pre = "[ERROR]";
			break;
		case FATAL:
			pre = "[FATAL]";
			break;
		case DEBUG:
			pre = "[DEBUG]";
			break;
		default:
			break;
	}

	std::cout << pre + Timestamp::now().toString() << " : " << msg << std::endl;
}

// int main(){
// 	Logger::instance().setLogLevel(INFO);
// 	Logger::instance().log("hello");
// }