#define WIN32_LEAN_AND_MEAN	//�����������ڵ�windows��
#include <WinSock2.h>
#include <windows.h>
#include <stdio.h>
#include <WS2tcpip.h>
enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
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
		else if (0 == strcmp(cmdBuf, "login"))
		{
			Login login;
			strcpy_s(login.userName, "lyd");
			strcpy_s(login.passWord, "lydmm");
			// 5 ���������������
			send(_sock, (const char*)&login, sizeof(Login), 0);
			// ���շ��������ص�����
			LoginResult loginRet = {};
			recv(_sock, (char*)&loginRet, sizeof(LoginResult), 0);
			printf("��¼�����%d\n", loginRet.result);
		}
		else if (0 == strcmp(cmdBuf, "logout"))
		{
			Logout logout;
			strcpy_s(logout.userName, "lyd");
			// 5 ���������������
			send(_sock, (const char*)&logout, sizeof(Logout), 0);
			// ���շ��������ص�����
			LogoutResult logoutRet = {};
			recv(_sock, (char*)&logoutRet, sizeof(LogoutResult), 0);
			printf("�ǳ������%d\n", logoutRet.result);
		}
		else
		{
			printf("��֧�ֵ�������������롣 \n");
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