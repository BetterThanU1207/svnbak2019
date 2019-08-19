#define WIN32_LEAN_AND_MEAN	//�����������ڵ�windows��
#include <WinSock2.h>
#include <windows.h>
#include <stdio.h>
#include <WS2tcpip.h>
#include <thread>

enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR
};
struct DataHeader
{
	short dataLength;
	short cmd;
};
//DataPackage�̳еķ�ʽ�ڹ��캯���г�ʼ��
struct Login : public DataHeader
{
	Login()
	{
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char passWord[32];
};

struct LoginResult : public DataHeader
{
	LoginResult()
	{
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}
	int result;
};

struct Logout : public DataHeader
{
	Logout()
	{
		dataLength = sizeof(Logout);
		cmd = CMD_LOGOUT;
	}
	char userName[32];
};

struct LogoutResult : public DataHeader
{
	LogoutResult()
	{
		dataLength = sizeof(LogoutResult);
		cmd = CMD_LOGOUT_RESULT;
		result = 0;
	}
	int result;
};

struct NewUserJoin : public DataHeader
{
	NewUserJoin()
	{
		dataLength = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		socketID = 0;
	}
	int socketID;
};
bool g_bRun = true;
void cmdThread( SOCKET _sock)
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
		else if (0 == strcmp(cmdBuf, "login"))
		{
			Login login;
			strcpy_s(login.userName, "lyd");
			strcpy_s(login.passWord, "lydmm");
			// 5 ���������������
			send(_sock, (const char*)&login, sizeof(Login), 0);
		}
		else if (0 == strcmp(cmdBuf, "logout"))
		{
			Logout logout;
			strcpy_s(logout.userName, "lyd");
			// 5 ���������������
			send(_sock, (const char*)&logout, sizeof(Logout), 0);
		}
		else
		{
			printf("��֧�ֵ�������������롣 \n");
		}
	}	
}

int processDeal(SOCKET _cSock)
{
	//������
	char szRecv[4096] = {};
	// 5 ���տͻ�������
	int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecv;
	if (nLen <= 0)
	{
		printf("��������Ͽ����ӣ����������\n");
		return -1;
	}
	// 6 ��������
	switch (header->cmd)
	{
		case CMD_LOGIN_RESULT:
			{
				recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
				LoginResult* login = (LoginResult*)szRecv;
				printf("�յ�������������Ϣ��CMD_LOGIN_RESULT ���ݣ�%d \n", login->result);
			}
			break;
		case CMD_LOGOUT_RESULT:
			{
				recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
				LogoutResult* logout = (LogoutResult*)szRecv;
				printf("�յ�������������Ϣ��CMD_LOGOUT_RESULT ���ݣ�%d \n", logout->result);
			}
			break;
		case CMD_NEW_USER_JOIN:
			{
				recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
				NewUserJoin* userJoin = (NewUserJoin*)szRecv;
				printf("�յ�������������Ϣ��CMD_NEW_USER_JOIN ���ݣ�%d \n", userJoin->socketID);
			}		
			break;
	}
	return 0;
}


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
	//�����߳�
	std::thread t1(cmdThread, _sock);
	t1.detach();//�����̷߳���
	while (g_bRun)
	{
		//������socket
		fd_set fdRead;
		//�������
		FD_ZERO(&fdRead);
		//��ֵ
		FD_SET(_sock, &fdRead);
		timeval t = { 1, 0 };
		int ret = select(_sock, &fdRead, 0, 0, &t);
		if (ret < 0)
		{
			printf("select�������1��\n");
			break;
		}
		if (FD_ISSET(_sock, &fdRead))
		{
			FD_CLR(_sock, &fdRead);

			if (-1 == processDeal(_sock))
			{
				printf("select�������2��\n");
				break;
			}			
		}
		//�߳�thread
		//printf("����ʱ�䴦������ҵ�񡣡�����\n");
		
		//Sleep(3000);
	}	
	// 7 closesocket �ر��׽���
	closesocket(_sock);
	//--------------
	//���windows socket����
	WSACleanup();
	printf("���˳������������\n");
	getchar();
	return 0;
}