#include <stdlib.h>
#include <iostream>
#include <thread>
#include <mutex>//��
#include <memory>
#include "CELLTimestamp.hpp"
#include "Alloctor.h"
#include "CELLObjectPoll.hpp"
using namespace std;

//ԭ�Ӳ��������ɷָ�Ĳ������������������ʱ��С�Ĳ�����λ
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


//˵�������̺߳����̶߳�������cout��ʹ�л��У���Ϊû���������û�����ʱ�Ტ��һ��
//������Ӧ��Ƶ��ʹ�ã�������������
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
}//��ռʽ

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
		//t[n].detach();//�����߳�
		t[n].join();//�����߳�
	}
	cout << tTime.getElapsedTimeInMilliSec() << endl;
	cout << "Hello,main thread." << endl;	

	//c++��׼������ָ���һ��
	//shared_ptr<int> b = make_shared<int>();
	return 0;

}