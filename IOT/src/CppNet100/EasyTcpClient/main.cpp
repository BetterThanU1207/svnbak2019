#include <stdio.h>
#include <thread>

#include "EasyTcpClient.hpp"

void cmdThread( EasyTcpClient* client)
{
	while (true)
	{
		// 3 ��������
		char cmdBuf[256] = {};
		scanf_s("%s", cmdBuf, 256);
		// 4 ��������
		if (0 == strcmp(cmdBuf, "exit"))
		{
			client->CloseSocket();
			printf("�˳�cmdThread�̡߳�\n");
			break;
		}
		else if (0 == strcmp(cmdBuf, "login"))
		{
			Login login;
			strcpy_s(login.userName, "lyd");
			strcpy_s(login.passWord, "lydmm");
			// 5 ���������������
			client->SendData(&login);
		}
		else if (0 == strcmp(cmdBuf, "logout"))
		{
			Logout logout;
			strcpy_s(logout.userName, "lyd");
			// 5 ���������������
			client->SendData(&logout);
		}
		else
		{
			printf("��֧�ֵ�������������롣 \n");
		}
	}	
}

int main()
{
	EasyTcpClient client;//�����������Ϳ������Ӷ��������
	client.ConnectServer("127.0.0.1", 4567);
	//�����߳�
	std::thread t1(cmdThread, &client);
	t1.detach();//�����̷߳���
	while (client.isRun())
	{
		client.OnRun();
	}	
	client.CloseSocket();
	printf("���˳������������\n");
	getchar();
	return 0;
}