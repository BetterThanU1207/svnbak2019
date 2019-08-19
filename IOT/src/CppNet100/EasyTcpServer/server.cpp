#define WIN32_LEAN_AND_MEAN	//�����������ڵ�windows��
#include <WinSock2.h>
#include <windows.h>
#include <iostream>
#include <WS2tcpip.h>
#include <vector>
//Ҫ��c\s�ֽ���һ�¡�����
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

struct LoginResult	: public DataHeader
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

std::vector<SOCKET> g_clients;

int processDeal(SOCKET _cSock)
{
	//������
	char szRecv[4096] = {};
	// 5 ���տͻ�������
	int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecv;
	if (nLen <= 0)
	{
		printf("�ͻ���<Socket=%d>���˳������������\n", _cSock);
		return -1;
	}
	// 6 ��������
	switch (header->cmd)
	{
	case CMD_LOGIN:
	{
		Login login = {};
		recv(_cSock, (char*)&login + sizeof(DataHeader), sizeof(Login) - sizeof(DataHeader), 0);
		printf("�յ��ͻ���<Socket=%d>����CMD_LOGIN ���ݳ��ȣ�%d,  userName=%s passWord=%s \n", _cSock, login.dataLength, login.userName, login.passWord);
		//�����ж��û��������Ƿ���ȷ�Ĺ���
		LoginResult ret;
		//�ȷ���Ϣͷ�ٷ���Ϣ��
		send(_cSock, (char*)&ret, sizeof(LoginResult), 0);
	}
	break;
	case CMD_LOGOUT:
	{
		Logout logout = {};
		recv(_cSock, (char*)&logout + sizeof(DataHeader), sizeof(Logout) - sizeof(DataHeader), 0);
		printf("�յ��ͻ���<Socket=%d>����CMD_LOGOUT ���ݳ��ȣ�%d,  userName=%s\n", _cSock, logout.dataLength, logout.userName);
		//�����ж��û��������Ƿ���ȷ�Ĺ���
		LogoutResult ret;
		//�ȷ���Ϣͷ�ٷ���Ϣ��
		send(_cSock, (char*)&ret, sizeof(LogoutResult), 0);
	}
	break;
	default:
		header->cmd = CMD_ERROR;
		header->dataLength = 0;
		send(_cSock, (char*)&header, sizeof(DataHeader), 0);
		break;
	}
	return 0;
}

int main()
{
	g_clients.clear();
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
	
	while (true)//ѭ���ظ������µĿͻ��ˣ�accept��ѭ����ʱ��/�ͻ���ָ��
	{
		//�������׽��� BSDsocket
		fd_set fdRead;//��������socket������
		fd_set fdWrite;
		fd_set fdExp;
		//���ϼ�������
		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);
		//��socket���뼯��
		FD_SET(_sock, &fdRead);
		FD_SET(_sock, &fdWrite);
		FD_SET(_sock, &fdExp);
		//size_t������--
		for (int n = (int)g_clients.size()-1; n >= 0; n--)
		{
			FD_SET(g_clients[n], &fdRead);
		}

		//nfds ��һ������ֵ ��ָfd_set������������������socket���ķ�Χ��������������
		//���������ļ����������ֵ+1 ��windows�������������д0
		//select���һ��������null��������ģʽ�������ݿɲ�����ʱ��ŷ��أ������������ݵķ�����Խ���
		timeval t = {1, 0};//��ѯʱ��Ϊ1 ����ѯʱ��Ϊ1���ǵȴ�1s ����������ģ��  �ۺ����������
		int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, &t);
		if (ret < 0)
		{
			printf("select���������\n");
			break;
		}
		//�ж��������Ƿ��ڼ�����
		if (FD_ISSET(_sock, &fdRead))
		{
			FD_CLR(_sock, &fdRead);
			// 4 accept �ȴ����տͻ�������
			sockaddr_in clientAddr = {};//�ͻ��˵�ַ
			int nAddrLen = sizeof(sockaddr_in);
			SOCKET _cSock = INVALID_SOCKET;

			_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
			if (INVALID_SOCKET == _cSock)
			{
				printf("���󣬽��յ���Ч�Ŀͻ���SOCKET������\n");
			}
			else
			{
				for (int n = (int)g_clients.size() - 1; n >= 0; n--)
				{
					NewUserJoin userJoin;
					userJoin.socketID = _cSock;
					send(g_clients[n], (const char*)&userJoin, sizeof(NewUserJoin), 0);
				}
				char sendBuf[20] = { '\0' };
				printf("�¿ͻ��˼��룺socket = %d, IP = %s \n", (int)_cSock, inet_ntop(AF_INET, (void*)&clientAddr.sin_addr, sendBuf, 16));
				g_clients.push_back(_cSock);
			}			
		}
		for (size_t n = 0; n < fdRead.fd_count; n++)
		{
			if (-1 == processDeal(fdRead.fd_array[n]))
			{
				auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[n]);
				if (iter != g_clients.end())
				{
					g_clients.erase(iter);
				}
			}
		}
		printf("����ʱ�䴦������ҵ�񡣡�����\n");
	}
	for (size_t n = g_clients.size() - 1; n >= 0; n--)
	{
		closesocket(g_clients[n]);
	}
	// 8 closesocket �ر��׽���
	closesocket(_sock);
	//--------------
	//���windows socket����
	WSACleanup();
	printf("���˳������������\n");
	getchar();
	return 0;
}