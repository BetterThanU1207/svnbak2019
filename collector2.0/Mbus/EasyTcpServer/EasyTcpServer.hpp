/*
文档说明：server socket 一个EasyTcpServer start n个CellServer，一个CellServer均分所有客户端
*/
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
#include "CELLNetWork.hpp"

//new 堆内存，直接声明的对象在栈上面
class EasyTcpServer : public INetEvent
{
private:
	//
	CELLThread _thread;
	//消息处理对象，内部会创建线程
	std::vector<CellServer*> _cellServers;
	//每秒消息计时
	CELLTimestamp _tTime;
	//
	SOCKET _sock;
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
		_recvCount = 0;
		_msgCount = 0;
		_clientCount = 0;
	}
	virtual ~EasyTcpServer()
	{
		CloseSocket();
		//_sock = INVALID_SOCKET;
		//_cellServers.clear();
	}
	//初始化socket
	SOCKET InitSocket()
	{
		CELLNetWork::Init();
		// 1 建立一个socket;Ipv4，面向数据流，TCP协议
		if (INVALID_SOCKET != _sock)
		{
			CELLLog::Info("warning, initSocket close old socket<%d>...\n", (int)_sock);
			CloseSocket();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			CELLLog::Info("error, create socket failed...\n");
		}
		else {
			CELLLog::Info("create socket<%d> success...\n", (int)_sock);
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
			CELLLog::Info("error, bind port<%d> failed...\n", port);
		}
		else {
			CELLLog::Info("bind port<%d> success...\n", port);
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
			CELLLog::Info("error, listen socket<%d> failed...\n",_sock);
		}
		else {
			CELLLog::Info("listen socket<%d> success...\n", _sock);
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
			CELLLog::Info("error, accept INVALID_SOCKET...\n");
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
		
	}
	void Start(int nCellServer)
	{
		for (int i = 0; i < nCellServer; i++)
		{
			auto ser = new CellServer(i+1);
			_cellServers.push_back(ser);
			//注册网络事件接收对象
			ser->setEventObj(this);
			//启动消息处理线程
			ser->Start();
		}
		_thread.Start(
			//onCreate
			nullptr,
			//onRun
			[this](CELLThread* pThread) {
				OnRun(pThread);
			}
		);
	}

	//关闭socket
	void CloseSocket()
	{
		CELLLog::Info("EasyTcpServer.CloseSocket begin\n");
		_thread.Close();
		if (INVALID_SOCKET != _sock)
		{
			for (auto s : _cellServers)
			{
				delete s;
			}
			_cellServers.clear();
			//closesocket 关闭套接字
#ifdef _WIN32			
			closesocket(_sock);
#else
			close(_sock);
#endif
			_sock = INVALID_SOCKET;
		}
		CELLLog::Info("EasyTcpServer.CloseSocket end\n");
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
private:
	//处理网络消息
	void OnRun(CELLThread* pThread)
	{
		while (pThread->isRun())
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
			timeval t = { 0, 1 };//查询时间为1 最大查询时间为1并非等待1s 非阻塞网络模型  综合性网络程序
			int ret = select(_sock + 1, &fdRead, 0, 0, &t);
			if (ret < 0)
			{
				CELLLog::Info("EasyTcpServer.OnRun select Error exit.\n");
				pThread->Exit();
				break;
			}
			//判断描述符是否在集合中
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);
				AcceptClient();
			}
		}
	}

	//计数并输出每秒收到的网络消息
	void time4msg()
	{
		auto t1 = _tTime.getElapsedSecond();
		if (t1 >= 1.0)
		{
			CELLLog::Info("thread<%d>,time<%lf>,socket<%d>, clients<%d>, recv<%d>, msg<%d>\n", _cellServers.size(), t1, _sock, (int)_clientCount, (int)(_recvCount / t1), (int)(_msgCount / t1));
			_tTime.update();
			_recvCount = 0;
			_msgCount = 0;
		}
	}

};

#endif // _EasyTcpServer_hpp_