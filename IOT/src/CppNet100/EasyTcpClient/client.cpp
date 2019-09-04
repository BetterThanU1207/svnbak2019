#include <stdio.h>
#include <thread>

#include "EasyTcpClient.hpp"
bool g_bRun = true;
void cmdThread( /*EasyTcpClient* client*/)
{
	while (true)
	{
		// 3 ��������
		char cmdBuf[256] = {};
		scanf_s("%s", cmdBuf, 256);
		// 4 ��������
		if (0 == strcmp(cmdBuf, "exit"))
		{
			g_bRun = false;
			//client->CloseSocket();
			printf("�˳�cmdThread�̡߳�\n");
			break;
		}
		//else if (0 == strcmp(cmdBuf, "login"))
		//{
		//	Login login;
		//	strcpy_s(login.userName, "lyd");
		//	strcpy_s(login.passWord, "lydmm");
		//	// 5 ���������������
		//	client->SendData(&login);
		//}
		//else if (0 == strcmp(cmdBuf, "logout"))
		//{
		//	Logout logout;
		//	strcpy_s(logout.userName, "lyd");
		//	// 5 ���������������
		//	client->SendData(&logout);
		//}
		else
		{
			printf("��֧�ֵ�������������롣 \n");
		}
	}	
}

//�ͻ�������
const int cCount = 10000;//windowsĬ�Ͽͻ�������������ȥһ������ˣ��������򲻻ᴫ������
//�߳�����
const int tCount = 4;
//��Ҫ��ָ�룬��Ȼջ�ڴ�ᱬ��
//�ͻ�������
EasyTcpClient* client[cCount];//�����������Ϳ������Ӷ��������

void sendThread(int id)
{	
	//4���߳� ID 1~4
	int c = cCount / tCount;
	int begin = (id - 1) * c;
	int end = id * c;

	for (int n = begin; n < end; n++)
	{
		client[n] = new EasyTcpClient();
	}
	for (int n = begin; n < end; n++)
	{
		printf("thread<%d>,Connect=%d\n", id, n);
		client[n]->ConnectServer("127.0.0.1", 4567);
	}

	//����
	std::chrono::milliseconds t(5000);
	std::this_thread::sleep_for(t);

	Login login[10];
	for (int n = 0; n < 10; n++)
	{
		strcpy_s(login[n].userName, "lyd");
		strcpy_s(login[n].passWord, "lydmm");
	}
	const int nLen = sizeof(login);
	while (g_bRun)
	{
		for (int n = begin; n < end; n++)
		{
			client[n]->SendData(login, nLen);
			//client[n]->OnRun();
		}
	}

	for (int n = begin; n < end; n++)
	{
		client[n]->CloseSocket();
	}
}

int main()
{
	//����UI�߳�
	std::thread t1(cmdThread);
	t1.detach();//�����̷߳���
	

	//���������߳�
	for (int i = 0; i < tCount; i++)
	{
		std::thread t1(sendThread, i + 1);
		t1.detach();
	}

	while (g_bRun)
	{
		Sleep(100);
	}
	
	printf("���˳������������\n");
	getchar();
	return 0;
}