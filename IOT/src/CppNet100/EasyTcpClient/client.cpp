#define WIN32_LEAN_AND_MEAN	//�����������ڵ�windows��
#include <WinSock2.h>
#include <windows.h>
#include <stdio.h>
#include <WS2tcpip.h>

//Ҫ��c\s�ֽ���һ�¡�����
struct DataPackage
{
	int age;
	char name[32];
};

int main()
{
	//����windows socket 2.x����
	WORD ver = MAKEWORD(2, 2);
	WSADATA data;
	WSAStartup(ver, &data);
	//-- ��socket api ��������TCP�ͻ���
	// 1 ����һ��socket;Ipv4��������������TCPЭ��
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == _sock)
	{
		printf("���󣬽���socketʧ�ܡ�����\n");
	}
	// 2 connect ���ӷ�����
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;//��������
	_sin.sin_port = htons(4567);//��ֹ�����е�short�����������ֽ����еĲ�ͬ
	//_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	inet_pton(AF_INET, "127.0.0.1", (void*)&_sin.sin_addr.S_un.S_addr);
	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret)
	{
		printf("�������ӷ�����ʧ�ܡ�����\n");
	}
	else {
		printf("���ӷ������ɹ�������\n");
	}
		
	while (true)
	{
		// 3 ��������
		char cmdBuf[128] = {};
		scanf_s("%s", cmdBuf, 128);
		// 4 ��������
		if (0 == strcmp(cmdBuf, "exit"))
		{
			printf("�յ��˳�������������");
			break;
		}
		else
		{
			// 5 ���������������
			send(_sock, cmdBuf, strlen(cmdBuf) + 1, 0);
		}
		// 6 recv ���շ�������Ϣ
		char recvBuf[256] = {};
		int nLen = recv(_sock, recvBuf, 256, 0);
		if (nLen > 0)
		{
			DataPackage* dp = (DataPackage*)recvBuf;
			printf("���յ����ݣ�����=%d������=%s \n", dp->age, dp->name);
		}
	}	
	// 7 closesocket �ر��׽���
	closesocket(_sock);
	//--------------
	//���windows socket����
	WSACleanup();
	printf("���˳������������");
	getchar();
	return 0;
}