#include <stdlib.h>
#include <iostream>
#include <thread>
#include <mutex>//锁
#include "CELLTimestamp.hpp"
//#inclu de "Alloctor.h"
using namespace std;

//原子操作（不可分割的操作）计算机处理命令时最小的操作单位
mutex m;
const int tCount = 4;
const int mCount = 100000;
const int nCount = mCount / tCount;
//说明：主线程和子线程都调用了cout即使有换行，因为没有锁定调用还是有时会并做一行
//锁：不应该频繁使用；合理锁定区域
void workFun(int index)
{
	char* data[nCount];
	for (size_t i = 0; i < nCount; i++)
	{
		data[i] = new char[ (rand() % 128) + 1];
	}
	for (size_t i = 0; i < nCount; i++)
	{
		delete[] data[i];
	}
	/*
	for (int n = 0; n < nCount; n++)
	{
		//自解锁 出作用域就unlock
		lock_guard<mutex> lg(m);
		//m.lock();//只锁定公共资源，一定要合理
		//临街区域-开始
		//临街区域-结束
		//m.unlock();		
	}*/
	//cout << index << "Hello,workFun thread." << n << endl;
}//抢占式

int main()
{
	thread t[tCount];
	for (int n = 0; n < tCount; n++)
	{
		t[n] = thread(workFun, n);
	}
	CELLTimestamp tTime;
	for (int n = 0; n < tCount; n++)
	{
		//t[n].detach();//启动线程
		t[n].join();//启动线程
	}
	cout << tTime.getElapsedTimeInMilliSec() << endl;
	cout << "Hello,main thread." << endl;
	return 0;
}