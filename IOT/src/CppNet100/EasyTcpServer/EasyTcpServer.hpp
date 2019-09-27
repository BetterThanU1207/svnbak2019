#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

//跨平台头文件
#ifdef _WIN32
	#define FD_SETSIZE			2506//突破windows下select64个的限制
	#define WIN32_LEAN_AND_MEAN	//避免引用早期的windows库
	#define _WINSOCK_DEPRECATED_NO_WARNINGS	//避免不能使用socket的旧函数出现错误
	#include <WS2tcpip.h>	//socket的新函数头文件
	#include <windows.h>
	#include <WinSock2.h>
	#pragma comment(lib, "ws2_32.lib")
#else
	#include <unistd.h>//uni std unix系统下的标准库
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
#include <functional>//mem_fun 安全转换
#include <atomic>
#include <memory>

#include "MessageHeader.hpp"
#include "CELLTimestamp.hpp"
#include "CELLTask.hpp"

//缓冲区最小单元大小
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240
#define SEND_BUFF_SIZE RECV_BUFF_SIZE
#endif

//客户端数据类型
class ClientSocket
{
public:
	ClientSocket(SOCKET sockfd = INVALID_SOCKET)
	{
		_sockfd = sockfd;
		memset(_szMsgBuf, 0, RECV_BUFF_SIZE);
		_lastPos = 0;

		memset(_szSendBuf, 0, SEND_BUFF_SIZE);
		_lastSendPos = 0;
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

	//发送数据
	int SendData(DataHeader* header)
	{
		int ret = SOCKET_ERROR;
		//要发送的数据长度
		int nSendLen = header->dataLength;
		//要发送的数据
		const char* pSendData = (const char*)header;
		while (true)
		{
			if (_lastSendPos + nSendLen >= SEND_BUFF_SIZE)
			{
				//计算可拷贝的数据长度
				int nCopyLen = SEND_BUFF_SIZE - _lastSendPos;
				//拷贝数据
				memcpy(_szSendBuf + _lastSendPos, pSendData, nCopyLen);
				//计算剩余数据位置
				pSendData += nCopyLen;
				//计算剩余数据长度
				nSendLen -= nCopyLen;
				//发送数据
				ret = send(_sockfd, _szSendBuf, SEND_BUFF_SIZE, 0);
				//数据尾部位置清零
				_lastSendPos = 0;
				//发送错误
				if (SOCKET_ERROR == ret)
				{
					return ret;
				}
			}
			else
			{
				//将要发送的数据 拷贝到发送缓冲区尾部
				memcpy(_szSendBuf + _lastSendPos, pSendData, nSendLen);
				//计算数据尾部位置
				_lastSendPos += nSendLen;
				break;
			}
		}

		return ret;
	}
private:
	//socket fd_set file desc set
	SOCKET _sockfd;
	//第二缓冲区 消息缓冲区
	char _szMsgBuf[RECV_BUFF_SIZE];
	//消息缓冲区的数据尾部位置
	int _lastPos;

	//第二缓冲区 发送缓冲区
	char _szSendBuf[SEND_BUFF_SIZE];
	//消息缓冲区的数据尾部位置
	int _lastSendPos;
};
typedef std::shared_ptr<ClientSocket> ClientSocketPtr;

//为INetEvent::OnNetMsg的使用做前置声明
class CellServer;
//网络事件接口
class INetEvent
{
public:
	//客户端加入事件
	virtual void OnNetJoin(ClientSocketPtr& pClient) = 0;
	//客户端离开事件
	virtual void OnNetLeave(ClientSocketPtr& pClient) = 0;//纯虚函数
	//客户端消息事件
	virtual void OnNetMsg(CellServer*pCellServer, ClientSocketPtr& pClient, DataHeader* header) = 0;
	//recv事件
	virtual void OnNetRecv(ClientSocketPtr& pClient) = 0;
private:

};

//网络消息发送任务
class CellS2CTask : public CellTask
{
	ClientSocketPtr _pClient;
	DataHeader* _pHeader;
public:
	CellS2CTask(ClientSocketPtr pClient, DataHeader* header)
	{
		_pClient = pClient;
		_pHeader = header;
	}

	//执行任务
	void doTask()
	{
		_pClient->SendData(_pHeader);
		delete _pHeader;
	}
};

typedef std::shared_ptr<CellS2CTask> CellS2CTaskPtr;
//网络消息接收服务类
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
	
	//判断是否工作中
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}
	//处理网络消息
	//备份客户socket fd_set
	fd_set _fdRead_bak;
	//客户列表是否有变化
	bool _clients_change;
	SOCKET _maxSock;
	void OnRun()
	{
		_clients_change = true;
		while (isRun())
		{		
			if (!_clientsBuff.empty())
			{
				//从缓冲队列里取出客户数据
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuff)
				{
					_clients[pClient->sockfd()] = pClient;
				}
				_clientsBuff.clear();
				_clients_change = true;
			}
			//如果没有需要处理的客户端，就跳过
			if (_clients.empty())
			{
				//休眠一毫秒
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}
			//伯克利套接字 BSDsocket
			fd_set fdRead;//描述符（socket）集合
			//fd_set fdWrite;
			//fd_set fdExp;
			//集合计数清零
			FD_ZERO(&fdRead);
			//FD_ZERO(&fdWrite);
			//FD_ZERO(&fdExp);
			//将服务端socket加入集合
			//FD_SET(_sock, &fdRead); 不应该两个地方对同一个socket查询
			//FD_SET(_sock, &fdWrite);
			//FD_SET(_sock, &fdExp);

			if (_clients_change)
			{
				_clients_change = false;
				//将描述符（socket）加入集合
				_maxSock = _clients.begin()->second->sockfd();
				for (auto iter : _clients)
				{
					//将客户端socket加入集合
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

			//nfds 是一个整数值 是指fd_set集合中所有描述符（socket）的范围，而不是数量，
			//既是所有文件描述符最大值+1 在windows中这个参数可以写0
			//select最后一个参数是null，是阻塞模式（有数据可操作的时候才返回），纯接收数据的服务可以接受
			//timeval t = { 1, 0 };//查询时间为1 最大查询时间为1并非等待1s 非阻塞网络模型  综合性网络程序\
			//只收数据，不需要主动查询实例
			int ret = select(_maxSock + 1, &fdRead, nullptr, nullptr, nullptr);
			if (ret < 0)
			{
				printf("select任务结束。\n");
				CloseSocket();
				return;
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
			std::vector<ClientSocketPtr> temp;
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

	//缓冲区
	//接收数据 处理粘包 拆分包
	int RecvData(ClientSocketPtr pClient)
	{
		char* szRecv = pClient->msgBuf() + pClient->getLastPos();
		// 5 接收数据
		int nLen = (int)recv(pClient->sockfd(), szRecv, RECV_BUFF_SIZE - pClient->getLastPos(), 0);
		_pNetEvent->OnNetRecv(pClient);
		if (nLen <= 0)
		{
			//printf("客户端<Socket=%d>已退出，任务结束。\n", pClient->sockfd());
			return -1;
		}
		//将收取的数据拷贝到消息缓冲区
		//memcpy(pClient->msgBuf() + pClient->getLastPos(), _szRecv, nLen);
		//消息缓冲区的数据尾部位置后移
		pClient->setLastPos(pClient->getLastPos() + nLen);
		//判断消息缓冲区的数据长度是否大于消息头DataHeader长度		
		//积压消息够多有可能会阻塞
		while (pClient->getLastPos() >= sizeof(DataHeader))//循环解决粘包
		{
			//这是就可以知道当前消息的长度
			DataHeader* header = (DataHeader*)pClient->msgBuf();
			//判断消息缓冲区的数据长度大于消息长度		
			if (pClient->getLastPos() >= header->dataLength)//判断解决少包   也可可能两者相等，那nSize=0
			{
				//消息缓冲区剩余未处理数据的长度   需要提前保存下来
				int nSize = pClient->getLastPos() - header->dataLength;//从接受缓冲区多取过来的一部分
				//处理网络消息
				OnNetMsg(pClient, header);//header被处理过后其中header被强制转换和位移已经改变
				//将消息缓冲区剩余未处理数据前移
				memcpy(pClient->msgBuf(), pClient->msgBuf() + header->dataLength, nSize);
				//消息缓冲区的数据尾部位置前移
				pClient->setLastPos(nSize);
			}
			else
			{
				//消息缓冲区剩余数据不够一条完整消息
				break;
			}
		}
		return 0;
	}
	//响应网络消息
	virtual void OnNetMsg(ClientSocketPtr pClient, DataHeader* header)
	{
		_pNetEvent->OnNetMsg(this, pClient, header);		
	}
	//关闭socket
	void CloseSocket()
	{
		if (INVALID_SOCKET != _sock)
		{
#ifdef _WIN32
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				closesocket(_clients[n]->sockfd());
			}
			//closesocket 关闭套接字
			closesocket(_sock);
#else
			for (auto iter : _clients)
			{
				close(iter.second->sockfd());
			}
			//closesocket 关闭套接字
			close(_sock);
#endif
		}
		_clients.clear();
	}

	void addClient(ClientSocketPtr pClient)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		//_mutex.lock();
		_clientsBuff.push_back(pClient);
		//_mutex.unlock();
	}

	void Start()
	{
		_thread = std::thread(std::mem_fn(&CellServer::OnRun), this);
		_taskServer.Start();
	}

	size_t getClientCount()
	{
		return _clients.size() + _clientsBuff.size();
	}

	void addSendTask(ClientSocketPtr pClient, DataHeader* header)
	{
		auto task = std::make_shared<CellS2CTask>(pClient, header);
		_taskServer.addTask((CellTaskPtr)task);
	}
private:
	SOCKET _sock;
	//正式客户队列
	std::map<SOCKET,ClientSocketPtr> _clients;
	//缓冲客户队列
	std::vector<ClientSocketPtr> _clientsBuff;
	//缓冲队列的锁
	std::mutex _mutex;
	std::thread _thread;
	//网络事件对象
	INetEvent* _pNetEvent;
	//
	CellTaskServer _taskServer;
};
typedef std::shared_ptr<CellServer> CellServerPtr;

//new 堆内存，直接声明的对象在栈上面
class EasyTcpServer : public INetEvent
{
private:
	SOCKET _sock;
	//消息处理对象，内部会创建线程
	std::vector<CellServerPtr> _cellServers;
	//每秒消息计时
	CELLTimestamp _tTime;
protected:
	//socket recv 计数
	std::atomic_int _recvCount;
	//收到消息计数
	std::atomic_int _msgCount;
	//客户端加入计数
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
	//初始化socket
	SOCKET InitSocket()
	{
#ifdef _WIN32
		//启动windows socket 2.x环境
		WORD ver = MAKEWORD(2, 2);
		WSADATA data;
		WSAStartup(ver, &data);
#endif
		//-- 用socket api 建立简易TCP客户端
		// 1 建立一个socket;Ipv4，面向数据流，TCP协议
		if (INVALID_SOCKET != _sock)
		{
			printf("<socket=%d>关闭旧连接。。。\n", (int)_sock);
			CloseSocket();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			printf("错误，建立socket失败。。。\n");
			//return -1;
		}
		else
		{
			printf("建立socket=<%d>成功。。。\n", (int)_sock);
		}
		return _sock;
	}
	//绑定ip和端口号
	int BindPort(const char* ip, unsigned short port)
	{
		//if (INVALID_SOCKET == _sock)
		//{
		//	if (-1 == InitSocket())
		//	{
		//		return -1;
		//	}
		//}
		// 2 bind 绑定用于接收客户端连接的网络端口
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;//网络类型
		_sin.sin_port = htons(port);//防止主机中的short类型与网络字节序中的不同

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
			printf("错误，绑定网络端口<%d>失败。。。\n", port);
		}
		else {
			//printf("绑定网络端口<%d>成功。。。\n", port);
		}
		return ret;
	}
	//监听端口号
	int ListenPort(int n)//等待连接数n
	{
		// 3 listen 监听网络端口
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret)
		{
			printf("socket=<%d>错误，监听网络端口失败。。。\n", (int)_sock);
		}
		else {
			//printf("socket=<%d>监听网络端口成功。。。\n", (int)_sock);
		}
		return ret;
	}
	//接收客户端连接
	SOCKET AcceptClient()
	{
		// 4 accept 等待接收客户端连接
		sockaddr_in clientAddr = {};//客户端地址
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET cSock = INVALID_SOCKET;
#ifdef _WIN32
		cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
		cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif // _WIN32			
		if (INVALID_SOCKET == cSock)
		{
			printf("socket=<%d>错误，接收到无效的客户端SOCKET。。。\n", (int)_sock);
		}
		else
		{			
			//将新客户端分配给客户数量最少的CellServer			
			addClient2CellServer(std::make_shared<ClientSocket>(cSock));
			//获取IP地址 inet_ntoa(clientAddr.sin_addr);
		}
		return cSock;
	}
	void addClient2CellServer(ClientSocketPtr pClient)
	{
		//查找客户端数量最少的CellServer消息处理对象
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
			auto ser = std::make_shared<CellServer> (_sock);
			_cellServers.push_back(ser);
			//注册网络事件接收对象
			ser->setEventObj(this);
			//启动消息处理线程
			ser->Start();
		}
	}

	//关闭socket
	void CloseSocket()
	{
		if (INVALID_SOCKET != _sock)
		{
#ifdef _WIN32

			// 8 closesocket 关闭套接字
			closesocket(_sock);
			//--------------
			//清除windows socket环境
			WSACleanup();
#else
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				close(_clients[n]->sockfd());
			}
			// 8 closesocket 关闭套接字
			close(_sock);
#endif
		}
	}

	//处理网络消息
	bool OnRun()
	{
		if (isRun())
		{
			time4msg();
			//伯克利套接字 BSDsocket
			fd_set fdRead;//描述符（socket）集合
			//fd_set fdWrite;
			//fd_set fdExp;
			//集合计数清零
			FD_ZERO(&fdRead);
			//FD_ZERO(&fdWrite);
			//FD_ZERO(&fdExp);
			//将服务端socket加入集合
			FD_SET(_sock, &fdRead);
			//FD_SET(_sock, &fdWrite);
			//FD_SET(_sock, &fdExp);

			//nfds 是一个整数值 是指fd_set集合中所有描述符（socket）的范围，而不是数量，
			//既是所有文件描述符最大值+1 在windows中这个参数可以写0
			//select最后一个参数是null，是阻塞模式（有数据可操作的时候才返回），纯接收数据的服务可以接受
			timeval t = { 0, 10 };//查询时间为1 最大查询时间为1并非等待1s 非阻塞网络模型  综合性网络程序
			int ret = select(_sock + 1, &fdRead, 0, 0, &t);
			if (ret < 0)
			{
				printf("Accept Select任务结束。\n");
				CloseSocket();
				return false;
			}
			//判断描述符是否在集合中
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);
				AcceptClient();
				return true;//只要有新连接暂时不处理数据
			}
			
			return true;
		}
		return false;
	}

	//判断是否工作中
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	//计数并输出每秒收到的网络消息
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
	//只被一个线程触发 安全
	virtual void OnNetJoin(ClientSocketPtr& pClient)
	{
		_clientCount++;
	}
	//cellserver 4 多个线程触发 不安全 如果只开启一个CellServer就是安全的
	virtual void OnNetLeave(ClientSocketPtr& pClient)
	{
		_clientCount--;
	}
	//cellserver 4 多个线程触发 不安全 如果只开启一个CellServer就是安全的
	virtual void OnNetMsg(CellServer* pCellServer, ClientSocketPtr& pClient, DataHeader* header)
	{
		_msgCount++;
	}

	virtual void OnNetRecv(ClientSocketPtr& pClient)
	{
		_recvCount++;
	}
};

#endif // _EasyTcpServer_hpp_