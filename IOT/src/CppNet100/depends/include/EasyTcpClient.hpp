#ifndef _EasyTcpClient_hpp_
#define _EasyTcpClient_hpp_

#include "CELL.hpp"
#include "CELLNetWork.hpp"
#include "CELLClient.hpp"

class EasyTcpClient
{
public:
	EasyTcpClient()
	{
	
	}
	//虚析构函数
	virtual ~EasyTcpClient()
	{
		CloseSocket();
	}
	//初始化socket
	int InitSocket()
	{
		CELLNetWork::Init();
		if (_pClient)
		{
			CELLLog::Info("<socket=%d>close old connect.....\n", _pClient->sockfd());
			CloseSocket();
		}
		SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
		if (INVALID_SOCKET == sock)
		{
			CELLLog::Info("error, create socket failed...\n");
			return -1;
		}
		else
		{
			//CELLLog::Info("create socket=<%d>success...\n", sock);
			_pClient = new CellClient(sock);
		}
		return 0;
	}
	//连接服务器
	int ConnectServer(const char* ip, unsigned short port)
	{
		if (!_pClient)
		{
			InitSocket();
		}
		// 2 connect 连接服务器
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;//网络类型
		_sin.sin_port = htons(port);//防止主机中的short类型与网络字节序中的不同
#ifdef _WIN32
		//_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
		inet_pton(AF_INET, ip, (void*)&_sin.sin_addr.S_un.S_addr);
#else
		//_sin.sin_addr.s_addr = inet_addr("127.0.0.1");//可以定义宏修正错误
		inet_pton(AF_INET, "127.0.0.1", (void*)&_sin.sin_addr.S_addr);
#endif

		int ret = connect(_pClient->sockfd(), (sockaddr*)&_sin, sizeof(sockaddr_in));
		//CELLLog::Info("<socket=%d>connecting server <%s:%d>。。。\n", (int)_sock, ip, port);
		if (SOCKET_ERROR == ret)
		{
			CELLLog::Info("<socket=%d>error，connect server<%s:%d>failed....\n", (int)_pClient->sockfd(), ip, port);
		}
		else {
			_isConnect = true;
			//CELLLog::Info("<socket=%d>connect server<%s:%d>success...\n", (int)_pClient->sockfd(), ip, port);
		}
		return ret;
	}

	//关闭socket
	void CloseSocket()
	{
		if (_pClient)
		{		
			delete _pClient;
			_pClient = nullptr;
		}
		_isConnect = false;
	}

	//查询网络消息
	bool OnRun()
	{
		if (isRun())
		{
			SOCKET sock = _pClient->sockfd();
			//伯克利socket
			fd_set fdRead;			
			//清空数组
			FD_ZERO(&fdRead);			
			//赋值
			FD_SET(sock, &fdRead);
			fd_set fdWrite;
			FD_ZERO(&fdWrite);

			timeval t = { 0, 1 };
			int ret = 0;
			if (_pClient->needWrite())
			{				
				FD_SET(sock, &fdWrite);
				ret = select(sock + 1, &fdRead, &fdWrite, nullptr, &t);
			}
			else
			{
				ret = select(sock + 1, &fdRead, nullptr, nullptr, &t);
			}

			if (ret < 0)
			{
				CELLLog::Info("error, <socket=%d>OnRun.select exit.\n", (int)sock);
				CloseSocket();//不关闭连接 onRun 会重复提醒
				return false;
			}
			if (FD_ISSET(sock, &fdRead))
			{
				if (-1 == RecvData(sock))
				{
					CELLLog::Info("error, <socket=%d>OnRun.RecvData exit.\n", (int)sock);
					CloseSocket();
					return false;
				}
			}

			if (FD_ISSET(sock, &fdWrite))
			{
				if (-1 == _pClient->SendDataReal())
				{
					CELLLog::Info("error, <socket=%d>OnRun.SendDataReal exit.\n", (int)sock);
					CloseSocket();
					return false;
				}
			}
			return true;
		}
		return false;
	}

	//判断是否工作中
	bool isRun()
	{
		return _pClient && _isConnect;
	}

	//接收数据 处理粘包 拆分包
	int RecvData(SOCKET _cSock)
	{
		int nLen = _pClient->RecvData();
		if (nLen > 0)
		{
			//循环 判断是否有消息需要处理
			while (_pClient->hasMsg())//循环解决粘包
			{
				//处理网络消息
				OnNetMsg(_pClient->front_msg());//header被处理过后其中header被强制转换和位移已经改变
				//移除消息队列（缓冲区）最前的一条数据
				_pClient->pop_front_msg();
			}
		}
		return nLen;
	}

	//响应网络消息
	virtual void OnNetMsg(netmsg_DataHeader* header) = 0;

	//发送数据
	int SendData(netmsg_DataHeader* header)
	{
		return _pClient->SendData(header);
	}

protected:
	CellClient* _pClient =  nullptr;
	bool _isConnect = false;
};

#endif//_EasyTcpClient_hpp_