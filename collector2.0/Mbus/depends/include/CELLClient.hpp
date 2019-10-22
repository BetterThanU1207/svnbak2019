#ifndef _CellClient_hpp_
#define _CellClient_hpp_

#include "CELL.hpp"
#include "CELLBuffer.hpp"
//客户端心跳检测死亡计时时间
#define CLIENT_HREAT_DEAD_TIME 60000
//在间隔指定时间后把发送缓冲区内缓存的消息数据发送给客户端
#define CLIENT_SEND_BUFF_TIME 200
//客户端数据类型
class CellClient
{
public:
	int id = -1;
	//所属server id
	int serverId = -1;
	//接收字节数
	int _recvBytes = 0;
public:
	CellClient(SOCKET sockfd = INVALID_SOCKET) :
		_sendBuff(SEND_BUFF_SIZE),
		_recvBuff(RECV_BUFF_SIZE)
	{
		static int n = 1;
		id = n++;
		_sockfd = sockfd;

		resetDTHeart();
		resetDTSend();
	} 
	~CellClient()
	{
		CELLLog::Info("s=%d CellClient%d.~CellClient 1\n", serverId, id);
		if (INVALID_SOCKET != _sockfd)
		{
#ifdef _WIN32
			closesocket(_sockfd);
#else
			close(_sockfd);
#endif
			_sockfd = INVALID_SOCKET;
		}		
	}
	SOCKET sockfd()
	{
		return _sockfd;
	}

	int RecvData()
	{
		_recvBytes = _recvBuff.read4socket(_sockfd);
		return _recvBytes;
	}

	bool hasMsg()
	{
		return _recvBuff.hasMsg();
	}

	char* front_msg()
	{
		return _recvBuff.data();
	}

	void pop_front_msg()
	{
		if (hasMsg())
			_recvBuff.pop(_recvBytes);
	}

	bool needWrite()
	{
		return _sendBuff.needWrite();
	}

	//立即将发送缓冲区的数据发送数据给客户端
	int SendDataReal()
	{
		resetDTSend();
		return _sendBuff.write2socket(_sockfd);
	}
	//缓冲区的控制根据业务需求的差异而调整
	//发送数据
	int SendData(const char* pData, int len)
	{
		if (_sendBuff.push(pData, len))
		{
			return len;
		}
		return SOCKET_ERROR;
	}

	void resetDTHeart()
	{
		_dtHeart = 0;
	}

	void resetDTSend()
	{
		_dtSend = 0;
	}
	//心跳检测
	bool checkHeart(time_t dt)
	{
		_dtHeart += dt;
		//CELLLog::Info("checkHeart dead:s=%d, time=%d\n", _sockfd, _dtHeart);
		if (_dtHeart >= CLIENT_HREAT_DEAD_TIME)
		{
			CELLLog::Info("checkHeart dead:s=%d, time=%d\n", _sockfd, _dtHeart);
			return true;
		}
		return false;
	}
	//定时发送消息检测
	bool checkSend(time_t dt)
	{
		_dtSend += dt;
		if (_dtSend >= CLIENT_SEND_BUFF_TIME)
		{
			//CELLLog::Info("checkSend:s=%d, time=%d\n", _sockfd, _dtSend);
			//立即将发送缓冲区的数据发送出去
			SendDataReal();
			//重置发送即时
			resetDTSend();
			return true;
		}
		return false;
	}
private:
	//socket fd_set file desc set
	SOCKET _sockfd;
	//接收消息缓冲区
	CELLBuffer _recvBuff;
	//发送缓冲区
	CELLBuffer _sendBuff;

	//心跳死亡计时
	time_t _dtHeart;
	//上次发送消息数据的时间
	time_t _dtSend;
	//发送缓冲区遇到写满情况计数
	int _sendBuffFullCount = 0;

};
#endif // !_CellClient_hpp_