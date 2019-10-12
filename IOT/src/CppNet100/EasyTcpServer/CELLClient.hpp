#ifndef _CellClient_hpp_
#define _CellClient_hpp_

#include "CELL.hpp"
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
public:
	CellClient(SOCKET sockfd = INVALID_SOCKET)
	{
		static int n = 1;
		id = n++;
		_sockfd = sockfd;
		memset(_szMsgBuf, 0, RECV_BUFF_SIZE);
		_lastPos = 0;

		memset(_szSendBuf, 0, SEND_BUFF_SIZE);
		_lastSendPos = 0;

		resetDTHeart();
		resetDTSend();
	} 
	~CellClient()
	{
		printf("s=%d CellClient%d.~CellClient 1\n", serverId, id);
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

	//立即将发送缓冲区的数据发送数据给客户端
	int SendDataReal()
	{
		int ret = 0;//没有数据发送
		if (_lastSendPos > 0 &&  INVALID_SOCKET != _sockfd)
		{
			ret = send(_sockfd, _szSendBuf, _lastSendPos, 0);
			//数据尾部位置清零
			_lastSendPos = 0;
			//
			_sendBuffFullCount = 0;
			//
			resetDTSend();
		}
		return ret;
	}
	//缓冲区的控制根据业务需求的差异而调整

	//发送数据
	int SendData(netmsg_DataHeader* header)
	{
		int ret = SOCKET_ERROR;
		//要发送的数据长度
		int nSendLen = header->dataLength;
		//要发送的数据
		const char* pSendData = (const char*)header;
		
		if (_lastSendPos + nSendLen <= SEND_BUFF_SIZE)
		{				
			//将要发送的数据 拷贝到发送缓冲区尾部
			memcpy(_szSendBuf + _lastSendPos, pSendData, nSendLen);
			//计算数据尾部位置
			_lastSendPos += nSendLen;
			if (_lastSendPos  == SEND_BUFF_SIZE)
			{
				_sendBuffFullCount++;
			}
			return nSendLen;
		}
		else
		{
			_sendBuffFullCount++;
		}
		return ret;
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
		//printf("checkHeart dead:s=%d, time=%d\n", _sockfd, _dtHeart);
		if (_dtHeart >= CLIENT_HREAT_DEAD_TIME)
		{
			printf("checkHeart dead:s=%d, time=%d\n", _sockfd, _dtHeart);
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
			//printf("checkSend:s=%d, time=%d\n", _sockfd, _dtSend);
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
	//第二缓冲区 消息缓冲区
	char _szMsgBuf[RECV_BUFF_SIZE];
	//消息缓冲区的数据尾部位置
	int _lastPos;

	//第二缓冲区 发送缓冲区
	char _szSendBuf[SEND_BUFF_SIZE];
	//消息缓冲区的数据尾部位置
	int _lastSendPos;
	//心跳死亡计时
	time_t _dtHeart;
	//上次发送消息数据的时间
	time_t _dtSend;
	//发送缓冲区遇到写满情况计数
	int _sendBuffFullCount = 0;
};
#endif // !_CellClient_hpp_