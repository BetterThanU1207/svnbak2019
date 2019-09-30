#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

#include <vector>
#include<map>
#include <thread>
#include <mutex>
#include <functional>//mem_fun 安全转换
#include <atomic>

#include "CELL.hpp"
#include "CELLClient.hpp"
#include "CELLServer.hpp"
#include "INetEvent.hpp"

//new 堆内存，直接声明的对象在栈上面
class EasyTcpServer : public INetEvent
{
private:
	SOCKET _sock;
	//消息处理对象，内部会创建线程
	std::vector<CellServer*> _cellServers;
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
			addClient2CellServer(new CellClient(cSock));
			//获取IP地址 inet_ntoa(clientAddr.sin_addr);
		}
		return cSock;
	}
	void addClient2CellServer(CellClient* pClient)
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
			auto ser = new CellServer(_sock);
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
	virtual void OnNetJoin(CellClient* pClient)
	{
		_clientCount++;
	}
	//cellserver 4 多个线程触发 不安全 如果只开启一个CellServer就是安全的
	virtual void OnNetLeave(CellClient* pClient)
	{
		_clientCount--;
	}
	//cellserver 4 多个线程触发 不安全 如果只开启一个CellServer就是安全的
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