#include <iostream>
#include <thread>
#include <mutex>//��
#include <atomic>//ԭ��
#include "CELLTimestamp.hpp"
using namespace std;

//ԭ�Ӳ��������ɷָ�Ĳ������������������ʱ��С�Ĳ�����λ
mutex m;
const int tCount = 4;

atomic<int> sum = 0;//���Ա�������ʹ��
//˵�������̺߳����̶߳�������cout��ʹ�л��У���Ϊû���������û�����ʱ�Ტ��һ��
//������Ӧ��Ƶ��ʹ�ã�������������
void workFun(int index)
{	
	for (int n = 0; n < 2000000; n++)
	{
		//�Խ��� ���������unlock
		lock_guard<mutex> lg(m);
		//m.lock();//ֻ����������Դ��һ��Ҫ����
		//�ٽ�����-��ʼ
		sum++;
		//�ٽ�����-����
		//m.unlock();		
	}	
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
	cout << tTime.getElapsedTimeInMilliSec()<<",sum=" << sum << endl;
	sum = 0;
	tTime.update();
	for (int n = 0; n < 8000000; n++)
	{
		sum++;
	}
	cout << tTime.getElapsedTimeInMilliSec() << ",sum=" << sum << endl;
	cout << "Hello,main thread." << endl;
	return 0;
}