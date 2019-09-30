#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

#include <vector>
#include<map>
#include <thread>
#include <mutex>
#include <functional>//mem_fun ��ȫת��
#include <atomic>

#include "CELL.hpp"
#include "CELLClient.hpp"
#include "CELLServer.hpp"
#include "INetEvent.hpp"

//new ���ڴ棬ֱ�������Ķ�����ջ����
class EasyTcpServer : public INetEvent
{
private:
	SOCKET _sock;
	//��Ϣ��������ڲ��ᴴ���߳�
	std::vector<CellServer*> _cellServers;
	//ÿ����Ϣ��ʱ
	CELLTimestamp _tTime;
protected:
	//socket recv ����
	std::atomic_int _recvCount;
	//�յ���Ϣ����
	std::atomic_int _msgCount;
	//�ͻ��˼������
	std::atomic_int _clientCount;


public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;
		_cellServers.clear();
		_recvCount = 0;
		_msgCount = 0;
		_clientCount = 0;
	}
	virtual ~EasyTcpServer()
	{
		CloseSocket();
		_sock = INVALID_SOCKET;
		_cellServers.clear();
	}
	//��ʼ��socket
	SOCKET InitSocket()
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
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			printf("���󣬽���socketʧ�ܡ�����\n");
			//return -1;
		}
		else
		{
			printf("����socket=<%d>�ɹ�������\n", (int)_sock);
		}
		return _sock;
	}
	//��ip�Ͷ˿ں�
	int BindPort(const char* ip, unsigned short port)
	{
		//if (INVALID_SOCKET == _sock)
		//{
		//	if (-1 == InitSocket())
		//	{
		//		return -1;
		//	}
		//}
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
		int ret = bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
		if (SOCKET_ERROR == ret)
		{
			printf("���󣬰�����˿�<%d>ʧ�ܡ�����\n", port);
		}
		else {
			//printf("������˿�<%d>�ɹ�������\n", port);
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
			//printf("socket=<%d>��������˿ڳɹ�������\n", (int)_sock);
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
			//���¿ͻ��˷�����ͻ��������ٵ�CellServer
			addClient2CellServer(new CellClient(cSock));
			//��ȡIP��ַ inet_ntoa(clientAddr.sin_addr);
		}
		return cSock;
	}
	void addClient2CellServer(CellClient* pClient)
	{
		//���ҿͻ����������ٵ�CellServer��Ϣ�������
		auto pMinServer = _cellServers[0];
		for (auto pCellServer : _cellServers)
		{
			if (pMinServer->getClientCount() > pCellServer->getClientCount())
			{
				pMinServer = pCellServer;
			}
		}
		pMinServer->addClient(pClient);
		OnNetJoin(pClient);
	}
	void Start(int nCellServer)
	{
		for (int i = 0; i < nCellServer; i++)
		{
			auto ser = new CellServer(_sock);
			_cellServers.push_back(ser);
			//ע�������¼����ն���
			ser->setEventObj(this);
			//������Ϣ�����߳�
			ser->Start();
		}
	}

	//�ر�socket
	void CloseSocket()
	{
		if (INVALID_SOCKET != _sock)
		{
#ifdef _WIN32

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
	}

	//����������Ϣ
	bool OnRun()
	{
		if (isRun())
		{
			time4msg();
			//�������׽��� BSDsocket
			fd_set fdRead;//��������socket������
			//fd_set fdWrite;
			//fd_set fdExp;
			//���ϼ�������
			FD_ZERO(&fdRead);
			//FD_ZERO(&fdWrite);
			//FD_ZERO(&fdExp);
			//�������socket���뼯��
			FD_SET(_sock, &fdRead);
			//FD_SET(_sock, &fdWrite);
			//FD_SET(_sock, &fdExp);

			//nfds ��һ������ֵ ��ָfd_set������������������socket���ķ�Χ��������������
			//���������ļ����������ֵ+1 ��windows�������������д0
			//select���һ��������null��������ģʽ�������ݿɲ�����ʱ��ŷ��أ������������ݵķ�����Խ���
			timeval t = { 0, 10 };//��ѯʱ��Ϊ1 ����ѯʱ��Ϊ1���ǵȴ�1s ����������ģ��  �ۺ����������
			int ret = select(_sock + 1, &fdRead, 0, 0, &t);
			if (ret < 0)
			{
				printf("Accept Select���������\n");
				CloseSocket();
				return false;
			}
			//�ж��������Ƿ��ڼ�����
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);
				AcceptClient();
				return true;//ֻҪ����������ʱ����������
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

	//���������ÿ���յ���������Ϣ
	void time4msg()
	{		
		auto t1 = _tTime.getElapsedSecond();
		if (t1  >= 1.0)
		{			
			printf("thread<%d>,time<%lf>,socket<%d>, clients<%d>, recv<%d>, msg<%d>\n",_cellServers.size(), t1, _sock, (int)_clientCount, (int)(_recvCount/t1), (int)(_msgCount / t1));
			_tTime.update();
			_recvCount = 0;
			_msgCount = 0;
		}		
	}
	//ֻ��һ���̴߳��� ��ȫ
	virtual void OnNetJoin(CellClient* pClient)
	{
		_clientCount++;
	}
	//cellserver 4 ����̴߳��� ����ȫ ���ֻ����һ��CellServer���ǰ�ȫ��
	virtual void OnNetLeave(CellClient* pClient)
	{
		_clientCount--;
	}
	//cellserver 4 ����̴߳��� ����ȫ ���ֻ����һ��CellServer���ǰ�ȫ��
	virtual void OnNetMsg(CellServer* pCellServer, CellClient* pClient, netmsg_DataHeader* header)
	{
		_msgCount++;
	}

	virtual void OnNetRecv(CellClient* pClient)
	{
		_recvCount++;
	}
};

#endif // _EasyTcpServer_hpp_