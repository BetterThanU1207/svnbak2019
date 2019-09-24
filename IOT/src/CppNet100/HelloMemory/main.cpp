#include <stdlib.h>
#include <iostream>
#include <thread>
#include <mutex>//��
#include "CELLTimestamp.hpp"
//#inclu de "Alloctor.h"
using namespace std;

//ԭ�Ӳ��������ɷָ�Ĳ������������������ʱ��С�Ĳ�����λ
mutex m;
const int tCount = 4;
const int mCount = 100000;
const int nCount = mCount / tCount;
//˵�������̺߳����̶߳�������cout��ʹ�л��У���Ϊû���������û�����ʱ�Ტ��һ��
//������Ӧ��Ƶ��ʹ�ã�������������
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
		//�Խ��� ���������unlock
		lock_guard<mutex> lg(m);
		//m.lock();//ֻ����������Դ��һ��Ҫ����
		//�ٽ�����-��ʼ
		//�ٽ�����-����
		//m.unlock();		
	}*/
	//cout << index << "Hello,workFun thread." << n << endl;
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
	return 0;
}