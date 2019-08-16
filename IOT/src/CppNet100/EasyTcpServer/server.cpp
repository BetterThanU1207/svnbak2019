#define WIN32_LEAN_AND_MEAN	//�����������ڵ�windows��
#include <WinSock2.h>
#include <windows.h>
#include <iostream>
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
	//-- ��socket api ��������TCP�����
	// 1 ����һ��socket;Ipv4��������������TCPЭ��
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == _sock)
	{
		printf("���󣬽���socketʧ�ܡ�����\n");
	}
	// 2 bind �����ڽ��տͻ������ӵ�����˿�
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;//��������
	_sin.sin_port = htons(4567);//��ֹ�����е�short�����������ֽ����еĲ�ͬ
	_sin.sin_addr.S_un.S_addr = INADDR_ANY; //inet_addr("127.0.0.1");
	//��һ����滻Ϊinet_pton(AF_INET, "127.0.0.1", (void*)&_sin.sin_addr.S_un.S_addr);
	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in)))
	{
		printf("���󣬰�����˿�ʧ�ܡ�����\n");
	}
	else {
		printf("������˿ڳɹ�������\n");
	}
	// 3 listen ��������˿�
	if (SOCKET_ERROR == listen(_sock, 5))
	{
		printf("���󣬼�������˿�ʧ�ܡ�����\n");
	}
	else {
		printf("��������˿ڳɹ�������\n");
	}
	// 4 accept �ȴ����տͻ�������
	sockaddr_in clientAddr = {};//�ͻ��˵�ַ
	int nAddrLen = sizeof(sockaddr_in);
	SOCKET _cSock = INVALID_SOCKET;
	
	_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
	if (INVALID_SOCKET == _cSock)
	{
		printf("���󣬽��յ���Ч�Ŀͻ���SOCKET������\n");
	}
	char sendBuf[20] = { '\0' };
	printf("�¿ͻ��˼��룺socket = %d, IP = %s \n", (int)_cSock, inet_ntop(AF_INET, (void*)&clientAddr.sin_addr, sendBuf, 16));

	char _recvBuf[128] = {};
	while (true)//ѭ���ظ������µĿͻ��ˣ�accept��ѭ����ʱ��/�ͻ���ָ��
	{
		// 5 ���տͻ�������
		int nLen = recv(_cSock, _recvBuf, 128, 0);
		if (nLen <= 0)
		{
			printf("�ͻ������˳������������\n");
			break;
		}
		printf("�յ����%s \n", _recvBuf);
		// 6 ��������
		if (0 == strcmp(_recvBuf, "getInfo"))
		{
			// 7 send ��ͻ��˷���һ������		
			DataPackage dp = { 80, "zhangsan" };
			send(_cSock, (const char*)&dp, sizeof(DataPackage), 0);
		}
		else
		{
			// 7 send ��ͻ��˷���һ������		
			char msgBuf[] = "???";
			send(_cSock, msgBuf, strlen(msgBuf) + 1, 0);
		}
	}
	// 8 closesocket �ر��׽���
	closesocket(_sock);
	//--------------
	//���windows socket����
	WSACleanup();
	printf("���˳������������");
	getchar();
	return 0;
}