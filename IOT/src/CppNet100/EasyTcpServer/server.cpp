#include "EasyTcpServer.hpp"
#include <thread>

bool g_bRun = true;
void cmdThread()
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
			printf("�˳�cmdThread�̡߳�\n");
			break;
		}
		else
		{
			printf("��֧�ֵ�������������롣 \n");
		}
	}
}
int main()
{
	EasyTcpServer server;
	server.InitSocket();
	server.BindPort(nullptr, 4567);
	server.ListenPort(5);
	server.Start();
	//�����߳�
	std::thread t1(cmdThread);
	t1.detach();//�����̷߳���
	while (g_bRun)//server.isRun()
	{
		server.OnRun();
	}
	server.CloseSocket();
	printf("���˳������������\n");
	getchar();
	return 0;
}