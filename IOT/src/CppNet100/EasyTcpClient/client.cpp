#include <stdio.h>
#include <thread>
#include <atomic>
#include "EasyTcpClient.hpp"
#include "CELLTimestamp.hpp"

bool g_bRun = true;
void cmdThread( /*EasyTcpClient* client*/)
{
	while (true)
	{
		// 3 输入请求
		char cmdBuf[256] = {};
		scanf_s("%s", cmdBuf, 256);
		// 4 处理请求
		if (0 == strcmp(cmdBuf, "exit"))
		{
			g_bRun = false;
			//client->CloseSocket();
			printf("退出cmdThread线程。\n");
			break;
		}
		//else if (0 == strcmp(cmdBuf, "login"))
		//{
		//	Login login;
		//	strcpy_s(login.userName, "lyd");
		//	strcpy_s(login.passWord, "lydmm");
		//	// 5 向服务器发送请求
		//	client->SendData(&login);
		//}
		//else if (0 == strcmp(cmdBuf, "logout"))
		//{
		//	Logout logout;
		//	strcpy_s(logout.userName, "lyd");
		//	// 5 向服务器发送请求
		//	client->SendData(&logout);
		//}
		else
		{
			printf("不支持的命令，请重新输入。 \n");
		}
	}	
}

//客户端数量
const int cCount = 10;//windows默认客户端最大个数，减去一个服务端，超出了则不会传输数据
//线程数量
const int tCount = 4;
//需要用指针，不然栈内存会爆掉
//客户端数组
EasyTcpClient* client[cCount];//声明多个对象就可以连接多个服务器
std::atomic_int sendCount = 0;
std::atomic_int readyCount = 0;

void sendThread(int id)
{	
	printf("thread<%d> start\n", id);
	//4个线程 ID 1~4
	int c = cCount / tCount;
	int begin = (id - 1) * c;
	int end = id * c;

	for (int n = begin; n < end; n++)
	{
		client[n] = new EasyTcpClient();
	}
	for (int n = begin; n < end; n++)
	{		
		client[n]->ConnectServer("127.0.0.1", 4567);
	}

	printf("thread<%d>,Connect<begin=%d, end=%d>\n", id, begin, end);

	readyCount++;
	while (readyCount < tCount)
	{
		//休眠  等待其他线程准备好发送数据 并发
		std::chrono::milliseconds t(10);
		std::this_thread::sleep_for(t);
	}

	Login login[1];
	for (int n = 0; n < 1; n++)
	{
		strcpy_s(login[n].userName, "lyd");
		strcpy_s(login[n].passWord, "lydmm");
	}
	const int nLen = sizeof(login);
	while (g_bRun)
	{
		for (int n = begin; n < end; n++)
		{
			if (SOCKET_ERROR != client[n]->SendData(login, nLen))
			{
				sendCount++;
			}			
			client[n]->OnRun();
		}
	}

	for (int n = begin; n < end; n++)
	{
		client[n]->CloseSocket();
		delete client[n];
	}
	printf("thread<%d> exit\n", id);
}

int main()
{
	//启动UI线程
	std::thread t1(cmdThread);
	t1.detach();//与主线程分离
	

	//启动发送线程
	for (int i = 0; i < tCount; i++)
	{
		std::thread t1(sendThread, i + 1);
		t1.detach();
	}

	CELLTimestamp tTime;

	while (g_bRun)
	{
		auto t = tTime.getElapsedSecond();
		if (t >= 1.0)
		{
			printf("thread<%d>, clients<%d>,time<%lf>,send<%d>\n", tCount, cCount, t, (int)(sendCount / t));
			tTime.update();
			sendCount = 0;
		}
		Sleep(1);
	}
	
	printf("已退出，任务结束。\n");
	getchar();
	return 0;
}