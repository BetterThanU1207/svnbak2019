#ifndef _EasyTcpClient_hpp_
#define _EasyTcpClient_hpp_
//��ƽ̨ͷ�ļ�
#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN	//�����������ڵ�windows��
	#include <windows.h>
	#include <WinSock2.h>
	#include <WS2tcpip.h>
	#pragma comment(lib, "ws2_32.lib")
#else
	#include <unistd.h>//uni std unixϵͳ�µı�׼��
	#include<arpa/inet.h>
	#include <string.h>

	#define  SOCKET int
	#define  INVALID_SOCKET		(SOCKET)(~0)
	#define  SOCKET_ERRROR						(-1)
#endif //_WIN32

#include <stdio.h>
#include "MessageHeader.hpp"

class EasyTcpClient
{
	SOCKET _sock;
public:
	EasyTcpClient()
	{
		_sock = INVALID_SOCKET;
	}
	//����������
	virtual ~EasyTcpClient()
	{
		CloseSocket();
	}
	//��ʼ��socket
	int InitSocket()
	{
#ifdef _WIN32
		//����windows socket 2.x����
		WORD ver = MAKEWORD(2, 2);
		WSADATA data;
		WSAStartup(ver, &data);
#endif
		//-- ��socket api ��������TCP�ͻ���
		// 1 ����һ��socket;Ipv4��������������TCPЭ��
		if (INVALID_SOCKET != _sock)
		{
			printf("<socket=%d>�رվ����ӡ�����\n", _sock);
			CloseSocket();
		}
		_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (INVALID_SOCKET == _sock)
		{
			printf("���󣬽���socketʧ�ܡ�����\n");
			return -1;
		}
		return 0;
	}
	//���ӷ�����
	void ConnectServer(char* ip, unsigned short port)
	{
		if (INVALID_SOCKET == _sock)
		{
			if (-1 == InitSocket())
			{
				return;
			}			
		}
		// 2 connect ���ӷ�����
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;//��������
		_sin.sin_port = htons(port);//��ֹ�����е�short�����������ֽ����еĲ�ͬ
#ifdef _WIN32
		//_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
		inet_pton(AF_INET, ip, (void*)&_sin.sin_addr.S_un.S_addr);
#else
		//_sin.sin_addr.s_addr = inet_addr("127.0.0.1");//���Զ������������
		inet_pton(AF_INET, "127.0.0.1", (void*)&_sin.sin_addr.S_addr);
#endif

		int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret)
		{
			printf("�������ӷ�����ʧ�ܡ�����\n");
		}
		else {
			printf("���ӷ������ɹ�������\n");
		}
	}

	//�ر�socket
	void CloseSocket()
	{
		if (INVALID_SOCKET != _sock)
		{		
			// 7 closesocket �ر��׽���
#ifdef _WIN32
		closesocket(_sock);
		//���windows socket����
		WSACleanup();
#else
		close(_sock);
#endif
		
		}
		_sock = INVALID_SOCKET;
	}

	//��ѯ������Ϣ
	bool OnRun()
	{
		if (isRun())
		{
			//������socket
			fd_set fdRead;
			//�������
			FD_ZERO(&fdRead);
			//��ֵ
			FD_SET(_sock, &fdRead);
			timeval t = { 1, 0 };
			//����ƽ̨��Ҫsock+1�������ղ����������Ϣ
			int ret = select(_sock + 1, &fdRead, 0, 0, &t);
			if (ret < 0)
			{
				printf("<socket=%d>select�������1��\n", _sock);
				return false;
			}
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);

				if (-1 == RecvData(_sock))
				{
					printf("<socket=%d>select�������2��\n", _sock);
					return false;
				}
			}
			return true;
		}
		return false;
	}

	//�ж��Ƿ�����
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	//�������� ����ճ�� ��ְ�
	int RecvData(SOCKET _cSock)
	{
		//������
		char szRecv[4096] = {};
		// 5 ���տͻ�������
		int nLen = (int)recv(_cSock, szRecv, sizeof(DataHeader), 0);
		DataHeader* header = (DataHeader*)szRecv;
		if (nLen <= 0)
		{
			printf("��������Ͽ����ӣ����������\n");
			return -1;
		}
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		OnNetMsg(header);
		return 0;
	}

	//��Ӧ������Ϣ
	void OnNetMsg(DataHeader* header)
	{
		// 6 ��������		
		switch (header->cmd)
		{
			case CMD_LOGIN_RESULT:
				{
					LoginResult* login = (LoginResult*)header;
					printf("�յ�������������Ϣ��CMD_LOGIN_RESULT ���ݣ�%d \n", login->result);
				}
				break;
			case CMD_LOGOUT_RESULT:
				{
					LogoutResult* logout = (LogoutResult*)header;
					printf("�յ�������������Ϣ��CMD_LOGOUT_RESULT ���ݣ�%d \n", logout->result);
				}
				break;
			case CMD_NEW_USER_JOIN:
				{
					NewUserJoin* userJoin = (NewUserJoin*)header;
					printf("�յ�������������Ϣ��CMD_NEW_USER_JOIN ���ݣ�%d \n", userJoin->socketID);
				}
				break;
		}
	}

	//��������
	int SendData(DataHeader* header)
	{
		if (isRun() && header)
		{
			return send(_sock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}
};

#endif//_EasyTcpClient_hpp_