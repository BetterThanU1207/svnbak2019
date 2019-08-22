#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

//��ƽ̨ͷ�ļ�
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN	//�����������ڵ�windows��
#define _WINSOCK_DEPRECATED_NO_WARNINGS	//���ⲻ��ʹ��socket�ľɺ������ִ���
#include <WS2tcpip.h>	//socket���º���ͷ�ļ�
#include <windows.h>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>//uni std unixϵͳ�µı�׼��
#include<arpa/inet.h>
#include <string.h>

#define  SOCKET int
#define  INVALID_SOCKET		(SOCKET)(~0)
#define  SOCKET_ERRROR						(-1)
#endif 
#include <stdio.h>
#include <vector>
#include "MessageHeader.hpp"

//��������С��Ԫ��С
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240
#endif

class ClientSocket
{
public:
	ClientSocket(SOCKET sockfd = INVALID_SOCKET)
	{
		_sockfd = sockfd;
		memset(_szMsgBuf, 0, sizeof(_szMsgBuf));
		_lastPos = 0;
	}
	SOCKET sockfd()
	{
		return _sockfd;
	}

	char* msgBuf()
	{
		return _szMsgBuf;
	}

	int getLastPos()
	{
		return _lastPos;
	}
	void setLastPos(int pos)
	{
		_lastPos = pos;
	}
private:
	//socket fd_set file desc set
	SOCKET _sockfd;
	//�ڶ������� ��Ϣ������
	char _szMsgBuf[RECV_BUFF_SIZE * 10];
	//��Ϣ������������β��λ��
	int _lastPos;
};

//new ���ڴ棬ֱ�������Ķ�����ջ����
class EasyTcpServer
{
private:
	SOCKET _sock;
	std::vector<ClientSocket*> _clients;//����ָ�루��̬�ڴ棩�������
public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;
	}
	virtual ~EasyTcpServer()
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
			printf("<socket=%d>�رվ����ӡ�����\n", (int)_sock);
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
			printf("����socket=<%d>�ɹ�������\n", (int)_sock);
		}
		return 0;
	}
	//��ip�Ͷ˿ں�
	int BindPort(const char* ip, unsigned short port)
	{
		if (INVALID_SOCKET == _sock)
		{
			if (-1 == InitSocket())
			{
				return -1;
			}
		}
		// 2 bind �����ڽ��տͻ������ӵ�����˿�
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;//��������
		_sin.sin_port = htons(port);//��ֹ�����е�short�����������ֽ����еĲ�ͬ

#ifdef _WIN32
		if (ip) 
		{
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		}
		else
		{
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;
		}		
#else
		if (ip)
		{
			_sin.sin_addr.s_addr = inet_addr(ip);
		}
		else
		{
			_sin.sin_addr.s_addr = INADDR_ANY;
		}
#endif
		int ret = bind(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret)
		{
			printf("���󣬰�����˿�<%d>ʧ�ܡ�����\n", port);
		}
		else {
			printf("������˿�<%d>�ɹ�������\n", port);
		}
		return ret;
	}
	//�����˿ں�
	int ListenPort(int n)//�ȴ�������n
	{
		// 3 listen ��������˿�
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret)
		{
			printf("socket=<%d>���󣬼�������˿�ʧ�ܡ�����\n", (int)_sock);
		}
		else {
			printf("socket=<%d>��������˿ڳɹ�������\n", (int)_sock);
		}
		return ret;
	}
	//���տͻ�������
	SOCKET AcceptClient()
	{
		// 4 accept �ȴ����տͻ�������
		sockaddr_in clientAddr = {};//�ͻ��˵�ַ
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET cSock = INVALID_SOCKET;
#ifdef _WIN32
		cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
		cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif // _WIN32			
		if (INVALID_SOCKET == cSock)
		{
			printf("socket=<%d>���󣬽��յ���Ч�Ŀͻ���SOCKET������\n", (int)_sock);
		}
		else
		{			
			NewUserJoin userJoin;
			SendData2All(&userJoin);
			char sendBuf[20] = { '\0' };
			printf("socket=<%d>�¿ͻ��˼��룺socket = %d, IP = %s \n", (int)_sock, (int)cSock, inet_ntop(AF_INET, (void*)&clientAddr.sin_addr, sendBuf, 16));
			_clients.push_back(new ClientSocket(cSock));
		}
		return cSock;
	}
	//�ر�socket
	void CloseSocket()
	{
		if (INVALID_SOCKET != _sock)
		{
#ifdef _WIN32
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				closesocket(_clients[n]->sockfd());
				delete _clients[n];//�ǰ�ȫɾ����Ӧ�п�
			}
			// 8 closesocket �ر��׽���
			closesocket(_sock);
			//--------------
			//���windows socket����
			WSACleanup();
#else
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				close(_clients[n]->sockfd());
			}
			// 8 closesocket �ر��׽���
			close(_sock);
#endif
		}
		_sock = INVALID_SOCKET;
		_clients.clear();
	}

	//��ѯ������Ϣ
	bool OnRun()
	{
		if (isRun())
		{
			//�������׽��� BSDsocket
			fd_set fdRead;//��������socket������
			fd_set fdWrite;
			fd_set fdExp;
			//���ϼ�������
			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);
			//�������socket���뼯��
			FD_SET(_sock, &fdRead);
			FD_SET(_sock, &fdWrite);
			FD_SET(_sock, &fdExp);

			SOCKET maxSock = _sock;
			//size_t������--
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				//���ͻ���socket���뼯��
				FD_SET(_clients[n]->sockfd(), &fdRead);
				if (maxSock < _clients[n]->sockfd())
				{
					maxSock = _clients[n]->sockfd();
				}
			}

			//nfds ��һ������ֵ ��ָfd_set������������������socket���ķ�Χ��������������
			//���������ļ����������ֵ+1 ��windows�������������д0
			//select���һ��������null��������ģʽ�������ݿɲ�����ʱ��ŷ��أ������������ݵķ�����Խ���
			timeval t = { 1, 0 };//��ѯʱ��Ϊ1 ����ѯʱ��Ϊ1���ǵȴ�1s ����������ģ��  �ۺ����������
			int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t);
			if (ret < 0)
			{
				printf("select���������\n");
				CloseSocket();
				return false;
			}
			//�ж��������Ƿ��ڼ�����
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);
				AcceptClient();
			}
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				if (FD_ISSET(_clients[n]->sockfd(), &fdRead))
				{
					if (-1 == RecvData(_clients[n]))
					{
						//�ͻ����˳��ڿͻ��˼�����ɾ���ͻ���
						auto iter = _clients.begin() + n;
						if (iter != _clients.end())
						{
							delete _clients[n];
							_clients.erase(iter);
						}
					}
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

	//������
	char _szRecv[RECV_BUFF_SIZE] = {};
	//�������� ����ճ�� ��ְ�
	int RecvData(ClientSocket* pClient)
	{		
		// 5 ��������
		int nLen = (int)recv(pClient->sockfd(), _szRecv, RECV_BUFF_SIZE, 0);
		if (nLen <= 0)
		{
			printf("�ͻ���<Socket=%d>���˳������������\n", pClient->sockfd());
			return -1;
		}
		//����ȡ�����ݿ�������Ϣ������
		memcpy(pClient->msgBuf() + pClient->getLastPos(), _szRecv, nLen);
		//��Ϣ������������β��λ�ú���
		pClient->setLastPos(pClient->getLastPos() + nLen);
		//�ж���Ϣ�����������ݳ����Ƿ������ϢͷDataHeader����		
		//��ѹ��Ϣ�����п��ܻ�����
		while (pClient->getLastPos() >= sizeof(DataHeader))//ѭ�����ճ��
		{
			//���ǾͿ���֪����ǰ��Ϣ�ĳ���
			DataHeader* header = (DataHeader*)pClient->msgBuf();
			//�ж���Ϣ�����������ݳ��ȴ�����Ϣ����		
			if (pClient->getLastPos() >= header->dataLength)//�жϽ���ٰ�   Ҳ�ɿ���������ȣ���nSize=0
			{
				//��Ϣ������ʣ��δ�������ݵĳ���   ��Ҫ��ǰ��������
				int nSize = pClient->getLastPos() - header->dataLength;//�ӽ��ܻ�������ȡ������һ����
				//����������Ϣ
				OnNetMsg(pClient->sockfd(), header);//header�������������header��ǿ��ת����λ���Ѿ��ı�
				//����Ϣ������ʣ��δ��������ǰ��
				memcpy(pClient->msgBuf(), pClient->msgBuf() + header->dataLength, nSize);
				//��Ϣ������������β��λ��ǰ��
				pClient->setLastPos(nSize);
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
	virtual void OnNetMsg(SOCKET cSock, DataHeader* header)
	{
		// 6 ��������
		switch (header->cmd)
		{
			case CMD_LOGIN:
				{
					Login* login = (Login*)header;
					//printf("�յ��ͻ���<Socket=%d>����CMD_LOGIN ���ݳ��ȣ�%d,  userName=%s passWord=%s \n", cSock, login->dataLength, login->userName, login->passWord);
					//�����ж��û��������Ƿ���ȷ�Ĺ���
					LoginResult ret;
					SendData(cSock, &ret);
				}
				break;
			case CMD_LOGOUT:
				{
					Logout* logout = (Logout*)header;
					//printf("�յ��ͻ���<Socket=%d>����CMD_LOGOUT ���ݳ��ȣ�%d,  userName=%s\n", cSock, logout->dataLength, logout->userName);
					//�����ж��û��������Ƿ���ȷ�Ĺ���
					LogoutResult ret;
					SendData(cSock, &ret);
				}
				break;
			default:
				{
					printf("<socket=%d>�յ�δ������Ϣ�����ݳ��ȣ�%d \n", (int)_sock, header->dataLength);
					//SendData(cSock, header);
				}				
				break;
		}
	}

	//���͸�ָ��socket������
	int SendData(SOCKET cSock, DataHeader* header)
	{
		if (isRun() && header)
		{
			return send(cSock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}
	//������socket��������
	void SendData2All(DataHeader* header)
	{
		for (int n = (int)_clients.size() - 1; n >= 0; n--)
		{
			SendData(_clients[n]->sockfd(), header);
		}
	}

};

#endif // _EasyTcpServer_hpp_