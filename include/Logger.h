#pragma once

#include <string>
#include "noncopyable.h"

//LOG_INFO("%s %d", arg1, arg2)	打印INFO级别的日志，使用do可以保证一定执行一次
//__VA_ARGS__是传递给宏的可变参数，...代表可变参数
#define LOG_INFO(logmsgFormat, ...)						\
	do{													\
		Logger &logger = Logger::instance();			\
		logger.setLogLevel(INFO);						\
		char buf[1024] = {0};							\
		snprintf(buf,1024,logmsgFormat,##__VA_ARGS__);	\
		logger.log(buf);								\
	}while(0)

//打印ERROR级别的日志
#define LOG_ERROR(logmsgFormat, ...)					\
	do{													\
		Logger &logger = Logger::instance();			\
		logger.setLogLevel(ERROR);						\
		char buf[1024] = {0};							\
		snprintf(buf,1024,logmsgFormat,##__VA_ARGS__);	\
		logger.log(buf);								\
	}while(0)

//打印FATAL级别的日志
#define LOG_FATAL(logmsgFormat, ...)					\
	do{													\
		Logger &logger = Logger::instance();			\
		logger.setLogLevel(FATAL);						\
		char buf[1024] = {0};							\
		snprintf(buf,1024,logmsgFormat,##__VA_ARGS__);	\
		logger.log(buf);								\
	}while(0)

//打印debug的日志
#ifdef MUDEBUG
#define LOG_DEBUG(logmsgFormat, ...)					\
	do{													\
		Logger &logger = Logger::instance();			\
		logger.setLogLevel(DEBUG);						\
		char buf[1024] = {0};							\
		snprintf(buf,1024,logmsgFormat,##__VA_ARGS__);	\
		logger.log(buf);								\
	}while(0)
#else
#define LOG_DEBUG(logmsgFormat, ...)
#endif
	
enum LogLevel{
	INFO, 	//普通信息
	ERROR,	//错误信息
	FATAL,	//core dump信息，可能导致程序崩溃
	DEBUG,	//调试信息
};

//输出一个日志类
class Logger : noncopyable{
	public:
		//获取日志唯一的实例对象引用，单例保证程序中只有一个实例
		static Logger &instance();
		//设置日志级别
		void setLogLevel(int level);
		//写日志
		void log(std::string msg);
	private:
		//日志等级
		int logLevel_;
};