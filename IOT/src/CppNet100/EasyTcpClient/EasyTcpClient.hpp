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
private:
	SOCKET _sock;
	bool _isConnect;
public:
	EasyTcpClient()
	{
		_sock = INVALID_SOCKET;
		_isConnect = false;
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
		else
		{
			//printf("����socket=<%d>�ɹ�������\n", _sock);
		}
		return 0;
	}
	//���ӷ�����
	int ConnectServer(const char* ip, unsigned short port)
	{
		if (INVALID_SOCKET == _sock)
		{
			if (-1 == InitSocket())
			{
				return -1;
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
		//printf("<socket=%d>�������ӷ�����<%s:%d>������\n", (int)_sock, ip, port);
		if (SOCKET_ERROR == ret)
		{
			printf("<socket=%d>�������ӷ�����<%s:%d>ʧ�ܡ�����\n", (int)_sock, ip, port);
		}
		else {
			_isConnect = true;
			//printf("<socket=%d>���ӷ�����<%s:%d>�ɹ�������\n", (int)_sock, ip, port);
		}
		return ret;
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
		_sock = INVALID_SOCKET;
		}
		_isConnect = false;
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
			timeval t = { 0, 0 };
			//����ƽ̨��Ҫsock+1�������ղ����������Ϣ
			int ret = select(_sock + 1, &fdRead, 0, 0, &t);
			if (ret < 0)
			{
				printf("<socket=%d>select�������1��\n", (int)_sock);
				CloseSocket();//���ر����� onRun ���ظ�����
				return false;
			}
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);

				if (-1 == RecvData(_sock))
				{
					printf("<socket=%d>select�������2��\n", (int)_sock);
					CloseSocket();
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
		return _sock != INVALID_SOCKET && _isConnect;
	}

	//��ʱ�ĳ�Ա����
	//��������С��Ԫ��С
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240
#endif

	//���ջ����� ��Ա��������ڹ��캯���г�ʼ��
	char _szRecv[RECV_BUFF_SIZE] = {};
	//�ڶ������� ��Ϣ������
	char _szMsgBuf[RECV_BUFF_SIZE] = {};
	//��Ϣ������������β��λ��
	int _lastPos = 0;
	//�������� ����ճ�� ��ְ�
	int RecvData(SOCKET _cSock)
	{
		// 5 ��������
		int nLen = (int)recv(_cSock, _szRecv, RECV_BUFF_SIZE, 0);
		if (nLen <= 0)
		{
			printf("<socket=%d>��������Ͽ����ӣ����������\n", (int)_sock);
			return -1;
		}
		//����ȡ�����ݿ�������Ϣ������
		memcpy(_szMsgBuf + _lastPos, _szRecv, nLen);
		//��Ϣ������������β��λ�ú���
		_lastPos += nLen;
		//�ж���Ϣ�����������ݳ����Ƿ������ϢͷDataHeader����		
		while (_lastPos >= sizeof(DataHeader))//ѭ�����ճ��
		{
			//���ǾͿ���֪����ǰ��Ϣ�ĳ���
			DataHeader* header = (DataHeader*)_szMsgBuf;
			//�ж���Ϣ�����������ݳ��ȴ�����Ϣ����		
			if (_lastPos >= header->dataLength)//�жϽ���ٰ�
			{
				//��Ϣ������ʣ��δ�������ݵĳ���   ��Ҫ��ǰ��������
				int nSize = _lastPos - header->dataLength;//�ӽ��ܻ�������ȡ������һ����
				//����������Ϣ
				OnNetMsg(header);//header�������������header��ǿ��ת���Ѿ��ı�
				//����Ϣ������ʣ��δ��������ǰ��
				memcpy(_szMsgBuf, _szMsgBuf + header->dataLength, nSize);
				//��Ϣ������������β��λ��ǰ��
				_lastPos = nSize;
			}
			else
			{
				//��Ϣ������ʣ�����ݲ���һ��������Ϣ
				break;
			}
		}
		return 0;
	}

	//��Ӧ������Ϣ
	virtual void OnNetMsg(DataHeader* header)
	{
		// 6 ��������		
		switch (header->cmd)
		{
			case CMD_LOGIN_RESULT:
				{
					LoginResult* login = (LoginResult*)header;
					//printf("<socket=%d>�յ�������������Ϣ��CMD_LOGIN_RESULT ���ݣ�%d \n", (int)_sock, login->result);
				}
				break;
			case CMD_LOGOUT_RESULT:
				{
					LogoutResult* logout = (LogoutResult*)header;
					//printf("<socket=%d>�յ�������������Ϣ��CMD_LOGOUT_RESULT ���ݣ�%d \n", (int)_sock, logout->result);
				}
				break;
			case CMD_NEW_USER_JOIN:
				{
					NewUserJoin* userJoin = (NewUserJoin*)header;
					//printf("<socket=%d>�յ�������������Ϣ��CMD_NEW_USER_JOIN ���ݣ�%d \n", (int)_sock, userJoin->socketID);
				}
				break;
			case CMD_ERROR:
				{
					printf("<socket=%d>�յ�������������Ϣ��CMD_ERROR�����ݳ��ȣ�%d \n", (int)_sock, header->dataLength);
				}
				break;
			default:
				{
					printf("<socket=%d>�յ�δ������Ϣ�����ݳ��ȣ�%d \n", (int)_sock, header->dataLength);
				}
				break;
		}
	}

	//��������
	int SendData(DataHeader* header,int nLen)
	{
		int ret = SOCKET_ERROR;
		if (isRun() && header)
		{
			ret = send(_sock, (const char*)header, nLen, 0);
			if (SOCKET_ERROR == ret)
			{
				CloseSocket();
			}
		}
		return ret;
	}
};

#endif//_EasyTcpClient_hpp_