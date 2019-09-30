#include <stdlib.h>
#include <iostream>
#include <thread>
#include <mutex>//锁
#include <memory>
#include "CELLTimestamp.hpp"
#include "Alloctor.h"
#include "CELLObjectPoll.hpp"
using namespace std;

//原子操作（不可分割的操作）计算机处理命令时最小的操作单位
mutex m;
const int tCount = 4;
const int mCount = 8;
const int nCount = mCount / tCount;

class ClassA : public ObjectPollBase<ClassA, 5>
{
public:
	ClassA(int n, char m)
	{
		num = n;
		a = m;
		printf("ClassA\n");
	}
	~ClassA()
	{
		printf("~ClassA\n");
	}

private:
	int num;
	char a;
};


//说明：主线程和子线程都调用了cout即使有换行，因为没有锁定调用还是有时会并做一行
//锁：不应该频繁使用；合理锁定区域
void workFun(int index)
{
	ClassA* data[nCount];
	for (size_t i = 0; i < nCount; i++)
	{
		data[i] = ClassA::createObject(i, 'a');
	}
	for (size_t i = 0; i < nCount; i++)
	{
		ClassA::destroyObject(data[i]);
	}
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

	//c++标准库智能指针的一种
	//shared_ptr<int> b = make_shared<int>();
	return 0;

}