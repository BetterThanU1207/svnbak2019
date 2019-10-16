﻿/*
文档说明：实现动态申请缓冲区及缓冲区管理
*/
#ifndef _CELL_BUFFER_HPP_
#define _CELL_BUFFER_HPP_

#include "CELL.hpp"
#include "DataFromCollector.h"

class CELLBuffer
{
public:
	CELLBuffer(int nSize = 8192)
	{
		_nSize = nSize;
		_pBuff = new char[_nSize];
	}
	~CELLBuffer()
	{
		if (_pBuff)
		{
			delete[] _pBuff;
			_pBuff = nullptr;
		}
	}

	char* data()
	{
		return _pBuff;
	}

	bool push(const char* pData, int nLen)
	{
		if (_nLast + nLen <= _nSize)
		{
			//将要发送的数据 拷贝到发送缓冲区尾部
			memcpy(_pBuff + _nLast, pData, nLen);
			//计算数据尾部位置
			_nLast += nLen;
			if (_nLast == SEND_BUFF_SIZE)
			{
				_fullCount++;
			}
			return true;
		}
		else
		{
			_fullCount++;
		}
		return false;
	}

	void pop(int nLen)
	{
		int n = _nLast - nLen;
		if (n > 0)
		{
			memcpy(_pBuff, _pBuff + nLen, n);			
		}
		_nLast = n;
		if (_fullCount > 0)
			--_fullCount;
	}

	int write2socket(SOCKET sockfd)
	{
		int ret = 0;//没有数据发送
		if (_nLast > 0 && INVALID_SOCKET != sockfd)
		{
			ret = send(sockfd, _pBuff, _nLast, 0);
			//数据尾部位置清零
			_nLast = 0;
			//
			_fullCount = 0;
		}
		return ret;
	}

	int read4socket(SOCKET sockfd)
	{
		if (_nSize - _nLast > 0)
		{
			//接收客户端数据
			char* szRecv = _pBuff + _nLast;
			int nLen = (int)recv(sockfd, szRecv, _nSize - _nLast, 0);
			//----------------解析协议-------------------------	
			DataFromCollector dataDeal;
			dataDeal.dataResult(szRecv, nLen);
			//打印原始数据的十六进制
			CELLLog::Info("rawData=");
			for (int i = 0; i< nLen; i++)
			{
				CELLLog::Info("%02X", szRecv[i]);		
				CELLLog::Info("\n");
			}			
			CELLLog::Info("\n");
			if (nLen <= 0)
			{
				return nLen;
			}
			//消息缓冲区的数据尾部位置后移
			_nLast += nLen;
			return nLen;
		}		
		return 0;
	}

	bool hasMsg()
	{
		//判断消息缓冲区的数据长度是否大于消息头netmsg_DataHeader长度		
		if (_nLast >= sizeof(netmsg_DataHeader))//循环解决粘包
		{
			//这是就可以知道当前消息的长度
			netmsg_DataHeader* header = (netmsg_DataHeader*)_pBuff;
			//判断消息缓冲区的数据长度大于消息长度		
			return _nLast >= header->dataLength;
		}
		return false;
	}

private:
	//第二缓冲区 发送缓冲区 动态大小
	char* _pBuff = nullptr;
	//可以用链表或者队列来管理缓冲数据块
	//list<char*> _pBuffList;
	//缓冲区的数据尾部位置，已有数据长度
	int _nLast = 0;
	//缓冲区总的空间大小，字节长度
	int _nSize = 0;
	//缓冲区写满次数计数
	int _fullCount = 0;
};


#endif // !_CELL_BUFFER_HPP_