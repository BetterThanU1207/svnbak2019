#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

//��ƽ̨ͷ�ļ�
#ifdef _WIN32
	#define FD_SETSIZE			2506//ͻ��windows��select64��������
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
#include<map>
#include <thread>
#include <mutex>
#include <functional>//mem_fun ��ȫת��
#include <atomic>
#include "MessageHeader.hpp"
#include "CELLTimestamp.hpp"

//��������С��Ԫ��С
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240
#endif

//�ͻ�����������
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

	//��������
	int SendData(DataHeader* header)
	{
		if (header)
		{//����
			return send(_sockfd, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}
private:
	//socket fd_set file desc set
	SOCKET _sockfd;
	//�ڶ������� ��Ϣ������
	char _szMsgBuf[RECV_BUFF_SIZE * 5];
	//��Ϣ������������β��λ��
	int _lastPos;
};

//�����¼��ӿ�
class INetEvent
{
public:
	//�ͻ��˼����¼�
	virtual void OnNetJoin(ClientSocket* pClient) = 0;
	//�ͻ����뿪�¼�
	virtual void OnNetLeave(ClientSocket* pClient) = 0;//���麯��
	//�ͻ�����Ϣ�¼�
	virtual void OnNetMsg(ClientSocket* pClient, DataHeader* header) = 0;
	//recv�¼�
	virtual void OnNetRecv(ClientSocket* pClient) = 0;
private:

};

class CellServer
{
public:
	CellServer(SOCKET sock = INVALID_SOCKET)
	{
		_sock = sock;
		_pNetEvent = nullptr;
	}
	~CellServer()
	{
		CloseSocket();
		_sock = INVALID_SOCKET;
	}
	
	void setEventObj(INetEvent* event)
	{
		_pNetEvent = event;
	}
	
	//�ж��Ƿ�����
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}
	//����������Ϣ
	//���ݿͻ�socket fd_set
	fd_set _fdRead_bak;
	//�ͻ��б��Ƿ��б仯
	bool _clients_change;
	SOCKET _maxSock;
	bool OnRun()
	{
		_clients_change = true;
		while (isRun())
		{		
			if (_clientsBuff.size() > 0)
			{
				//�ӻ��������ȡ���ͻ�����
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuff)
				{
					_clients[pClient->sockfd()] = pClient;
				}
				_clientsBuff.clear();
				_clients_change = true;
			}
			//���û����Ҫ����Ŀͻ��ˣ�������
			if (_clients.empty())
			{
				//����һ����
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}
			//�������׽��� BSDsocket
			fd_set fdRead;//��������socket������
			//fd_set fdWrite;
			//fd_set fdExp;
			//���ϼ�������
			FD_ZERO(&fdRead);
			//FD_ZERO(&fdWrite);
			//FD_ZERO(&fdExp);
			//�������socket���뼯��
			//FD_SET(_sock, &fdRead); ��Ӧ�������ط���ͬһ��socket��ѯ
			//FD_SET(_sock, &fdWrite);
			//FD_SET(_sock, &fdExp);

			if (_clients_change)
			{
				_clients_change = false;
				//����������socket�����뼯��
				_maxSock = _clients.begin()->second->sockfd();
				for (auto iter : _clients)
				{
					//���ͻ���socket���뼯��
					FD_SET(iter.second->sockfd(), &fdRead);
					if (_maxSock < iter.second->sockfd())
					{
						_maxSock = iter.second->sockfd();
					}
				}
				memcpy(&_fdRead_bak,  &fdRead, sizeof(fd_set));
			}
			else
			{
				memcpy(&fdRead, &_fdRead_bak, sizeof(fd_set));
			}

			//nfds ��һ������ֵ ��ָfd_set������������������socket���ķ�Χ��������������
			//���������ļ����������ֵ+1 ��windows�������������д0
			//select���һ��������null��������ģʽ�������ݿɲ�����ʱ��ŷ��أ������������ݵķ�����Խ���
			//timeval t = { 1, 0 };//��ѯʱ��Ϊ1 ����ѯʱ��Ϊ1���ǵȴ�1s ����������ģ��  �ۺ����������\
			//ֻ�����ݣ�����Ҫ������ѯʵ��
			int ret = select(_maxSock + 1, &fdRead, nullptr, nullptr, nullptr);
			if (ret < 0)
			{
				printf("select���������\n");
				CloseSocket();
				return false;
			}
			else if (ret == 0)
			{
				continue;
			}
			
#ifdef _WIN32
			for (int n = 0; n < fdRead.fd_count; n++)
			{
				auto iter  = _clients.find(fdRead.fd_array[n]);
				if (iter != _clients.end())
				{
					if (-1 == RecvData(iter->second))
					{
						if (_pNetEvent)
							_pNetEvent->OnNetLeave(iter->second);
						_clients_change = true;
						_clients.erase(iter->first);
					}
				}else {
					printf("error. if (iter != _clients.end())\n");
				}

			}
#else
			std::vector<ClientSocket*> temp;
			for (auto iter : _clients)
			{
				if (FD_ISSET(iter.second->sockfd(), &fdRead))
				{
					if (-1 == RecvData(iter.second))
					{
						if (_pNetEvent)
							_pNetEvent->OnNetLeave(iter.second);
						_clients_change = false;
						temp.push_back(iter.second);
					}
				}
			}
			for (auto pClient : temp)
			{
				_clients.erase(pClient->sockfd());
				delete pClient;
			}
#endif
		}
	}

	//������
	char _szRecv[RECV_BUFF_SIZE] = {};
	//�������� ����ճ�� ��ְ�
	int RecvData(ClientSocket* pClient)
	{
		// 5 ��������
		int nLen = (int)recv(pClient->sockfd(), _szRecv, RECV_BUFF_SIZE, 0);
		_pNetEvent->OnNetRecv(pClient);
		if (nLen <= 0)
		{
			//printf("�ͻ���<Socket=%d>���˳������������\n", pClient->sockfd());
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
				OnNetMsg(pClient, header);//header�������������header��ǿ��ת����λ���Ѿ��ı�
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
	virtual void OnNetMsg(ClientSocket* pClient, DataHeader* header)
	{
		_pNetEvent->OnNetMsg(pClient, header);		
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
			//closesocket �ر��׽���
			closesocket(_sock);
#else
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				close(_clients[n]->sockfd());
			}
			//closesocket �ر��׽���
			close(_sock);
#endif
		}
		_clients.clear();
	}

	void addClient(ClientSocket* pClient)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		//_mutex.lock();
		_clientsBuff.push_back(pClient);
		//_mutex.unlock();
	}

	void Start()
	{
		_thread = std::thread(std::mem_fn(&CellServer::OnRun), this);
	}

	size_t getClientCount()
	{
		return _clients.size() + _clientsBuff.size();
	}
private:
	SOCKET _sock;
	//��ʽ�ͻ�����
	std::map<SOCKET,ClientSocket*> _clients;
	//����ͻ�����
	std::vector<ClientSocket*> _clientsBuff;
	//������е���
	std::mutex _mutex;
	std::thread _thread;
	//�����¼�����
	INetEvent* _pNetEvent;
};

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
			addClient2CellServer(new ClientSocket(cSock));
			//��ȡIP��ַ inet_ntoa(clientAddr.sin_addr);
		}
		return cSock;
	}
	void addClient2CellServer(ClientSocket* pClient)
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
	virtual void OnNetJoin(ClientSocket* pClient)
	{
		_clientCount++;
	}
	//cellserver 4 ����̴߳��� ����ȫ ���ֻ����һ��CellServer���ǰ�ȫ��
	virtual void OnNetLeave(ClientSocket* pClient)
	{
		_clientCount--;
	}
	//cellserver 4 ����̴߳��� ����ȫ ���ֻ����һ��CellServer���ǰ�ȫ��
	virtual void OnNetMsg(ClientSocket* pClient, DataHeader* header)
	{
		_recvCount++;
	}
};

#endif // _EasyTcpServer_hpp_