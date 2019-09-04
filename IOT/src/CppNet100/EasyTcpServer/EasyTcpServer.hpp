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
#include <thread>
#include <mutex>
#include <functional>//mem_fun 安全转换
#include <atomic>
#include "MessageHeader.hpp"
#include "CELLTimestamp.hpp"

//缓冲区最小单元大小
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240
#endif

#define _CELL_THREAD_COUNT 4

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
	//第二缓冲区 消息缓冲区
	char _szMsgBuf[RECV_BUFF_SIZE * 10];
	//消息缓冲区的数据尾部位置
	int _lastPos;
};

class INetEvent
{
public:
	//纯虚函数
	//客户端离开事件
	virtual void OnLeave(ClientSocket* pClient) = 0;//纯虚函数
	virtual void OnNetMsg(SOCKET cSock, DataHeader* header) = 0;
private:

};

class CellServer
{
public:
	CellServer(SOCKET sock = INVALID_SOCKET)
	{
		_sock = sock;
		_pThread = nullptr;
		_recvCount = 0;
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
	bool OnRun()
	{
		while (isRun())
		{		
			if (_clientsBuff.size() > 0)
			{
				//从缓冲队列里取出客户数据
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuff)
				{
					_clients.push_back(pClient);
				}
				_clientsBuff.clear();
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

			SOCKET maxSock = _clients[0]->sockfd();
			//size_t不能做--
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				//将客户端socket加入集合
				FD_SET(_clients[n]->sockfd(), &fdRead);
				if (maxSock < _clients[n]->sockfd())
				{
					maxSock = _clients[n]->sockfd();
				}
			}
			//nfds 是一个整数值 是指fd_set集合中所有描述符（socket）的范围，而不是数量，
			//既是所有文件描述符最大值+1 在windows中这个参数可以写0
			//select最后一个参数是null，是阻塞模式（有数据可操作的时候才返回），纯接收数据的服务可以接受
			//timeval t = { 1, 0 };//查询时间为1 最大查询时间为1并非等待1s 非阻塞网络模型  综合性网络程序\
			//只收数据，不需要主动查询实例
			int ret = select(maxSock + 1, &fdRead, nullptr, nullptr, nullptr);
			if (ret < 0)
			{
				printf("select任务结束。\n");
				CloseSocket();
				return false;
			}

			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				if (FD_ISSET(_clients[n]->sockfd(), &fdRead))
				{
					if (-1 == RecvData(_clients[n]))
					{
						//客户端退出在客户端集合中删除客户端
						auto iter = _clients.begin() + n;
						if (iter != _clients.end())
						{
							if (_pNetEvent)
							{
								_pNetEvent->OnLeave(_clients[n]);
							}
							delete _clients[n];
							_clients.erase(iter);
						}
					}
				}
			}
		}
	}

	//缓冲区
	char _szRecv[RECV_BUFF_SIZE] = {};
	//接收数据 处理粘包 拆分包
	int RecvData(ClientSocket* pClient)
	{
		// 5 接收数据
		int nLen = (int)recv(pClient->sockfd(), _szRecv, RECV_BUFF_SIZE, 0);
		if (nLen <= 0)
		{
			printf("客户端<Socket=%d>已退出，任务结束。\n", pClient->sockfd());
			return -1;
		}
		//将收取的数据拷贝到消息缓冲区
		memcpy(pClient->msgBuf() + pClient->getLastPos(), _szRecv, nLen);
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
				OnNetMsg(pClient->sockfd(), header);//header被处理过后其中header被强制转换和位移已经改变
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
	virtual void OnNetMsg(SOCKET cSock, DataHeader* header)
	{
		_recvCount++;
		_pNetEvent->OnNetMsg(cSock, header);
		/*auto t1 = _tTime.getElapsedSecond();
		if (t1 >= 1.0)
		{
			printf("time<%lf>,socket<%d>, clients<%d>, recvCount<%d>\n", t1, _sock, _clients.size(), _recvCount);
			_recvCount = 0;
			_tTime.update();
		}*/
		// 6 处理请求
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			Login* login = (Login*)header;
			//printf("收到客户端<Socket=%d>请求：CMD_LOGIN 数据长度：%d,  userName=%s passWord=%s \n", cSock, login->dataLength, login->userName, login->passWord);
			//忽略判断用户名密码是否正确的过程
			//LoginResult ret;
			//SendData(cSock, &ret);
		}
		break;
		case CMD_LOGOUT:
		{
			Logout* logout = (Logout*)header;
			//printf("收到客户端<Socket=%d>请求：CMD_LOGOUT 数据长度：%d,  userName=%s\n", cSock, logout->dataLength, logout->userName);
			//忽略判断用户名密码是否正确的过程
			//LogoutResult ret;
			//SendData(cSock, &ret);
		}
		break;
		default:
		{
			printf("<socket=%d>收到未定义消息，数据长度：%d \n", (int)_sock, header->dataLength);
			//SendData(cSock, header);
		}
		break;
		}
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
				delete _clients[n];//非安全删除，应判空
			}
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
		_sock = INVALID_SOCKET;
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
		_pThread = new std::thread(std::mem_fun(&CellServer::OnRun), this);
	}

	size_t getClientCount()
	{
		return _clients.size() + _clientsBuff.size();
	}
private:
	SOCKET _sock;
	//正式客户队列
	std::vector<ClientSocket*> _clients;//创建指针（动态内存）不会崩溃
	//客户端缓冲队列
	std::vector<ClientSocket*> _clientsBuff;
	std::mutex _mutex;
	std::thread* _pThread;
	INetEvent* _pNetEvent;
public:
	std::atomic_int _recvCount;
};

//new 堆内存，直接声明的对象在栈上面
class EasyTcpServer : public INetEvent
{
private:
	SOCKET _sock;
	std::vector<ClientSocket*> _clients;//创建指针（动态内存）不会崩溃
	std::vector<CellServer*> _cellServers;
	CELLTimestamp _tTime;
public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;
		_clients.clear();
		_cellServers.clear();
	}
	virtual ~EasyTcpServer()
	{
		CloseSocket();
		_sock = INVALID_SOCKET;
		_clients.clear();
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
		_sock = socket(AF_INET, SOCK_STREAM, 0);
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
		int ret = bind(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
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
			//NewUserJoin userJoin;
			//SendData2All(&userJoin);
			//char sendBuf[20] = { '\0' };
			//printf("socket=<%d>新客户端<%d>加入：socket = %d, IP = %s \n", (int)_sock, _clients.size(), (int)cSock, inet_ntop(AF_INET, (void*)&clientAddr.sin_addr, sendBuf, 16));
			addClient2CellServer(new ClientSocket(cSock));
		}
		return cSock;
	}
	void addClient2CellServer(ClientSocket* pClient)
	{
		_clients.push_back(pClient);
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
	}
	void Start()
	{
		for (int i = 0; i < _CELL_THREAD_COUNT; i++)
		{
			auto ser = new CellServer(_sock);
			_cellServers.push_back(ser);
			ser->setEventObj(this);
			ser->Start();
		}
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
				delete _clients[n];//非安全删除，应判空
			}
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
		_sock = INVALID_SOCKET;
		_clients.clear();
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
				printf("select任务结束。\n");
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

	//响应网络消息
	void time4msg()
	{		
		auto t1 = _tTime.getElapsedSecond();
		if (t1  >= 1.0)
		{
			int recvCount = 0;
			for (auto ser : _cellServers)
			{
				recvCount += ser->_recvCount;
				ser->_recvCount = 0;
			}
			printf("thread<%d>,time<%lf>,socket<%d>, clients<%d>, recvCount<%d>\n",_cellServers.size(), t1, _sock, (int)_clients.size(), (int)(recvCount/t1));
			_tTime.update();
		}		
	}

	//发送给指定socket的数据
	int SendData(SOCKET cSock, DataHeader* header)
	{
		if (isRun() && header)
		{
			return send(cSock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}
	//向所有socket发送数据
	void SendData2All(DataHeader* header)
	{
		for (int n = (int)_clients.size() - 1; n >= 0; n--)
		{
			SendData(_clients[n]->sockfd(), header);
		}
	}


	virtual void OnLeave(ClientSocket* pClient)
	{
		for (int n = (int)_clients.size() - 1; n >= 0; n--)
		{
			if (_clients[n] == pClient)
			{
				auto iter = _clients.begin() + n;
				if (iter != _clients.end())
					_clients.erase(iter);
			}
		}
	}
	virtual void OnNetMsg(SOCKET cSock, DataHeader* header)
	{

	}
};

#endif // _EasyTcpServer_hpp_