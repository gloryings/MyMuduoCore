#pragma once

#include<iostream>
#include<string>

class Timestamp{
	public:
		Timestamp();
		explicit Timestamp(int64_t microSecondsSinceEpoch);	//禁止隐式转换
		static Timestamp now();			//静态成员函数，可在不创建对象下直接调用
		std::string toString() const;	//不修改类的成员变量值
	private:
		int64_t microSecondsSinceEpoch_;	//存储时间戳
};