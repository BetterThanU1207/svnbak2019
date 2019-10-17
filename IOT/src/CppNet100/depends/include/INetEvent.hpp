﻿#ifndef _I_NET_EVENT_HPP_
#define _I_NET_EVENT_HPP_

#include "CELL.hpp"
#include "CELLClient.hpp"

//为INetEvent::OnNetMsg的使用做前置声明  两个类互相嵌套只能使用前置声明
class CellServer;
//网络事件接口
class INetEvent
{
public:
	//客户端加入事件
	virtual void OnNetJoin(CellClient* pClient) = 0;
	//客户端离开事件
	virtual void OnNetLeave(CellClient* pClient) = 0;//纯虚函数
	//客户端消息事件
	virtual void OnNetMsg(CellServer* pCellServer, CellClient* pClient, netmsg_DataHeader* header) = 0;
	//recv事件
	virtual void OnNetRecv(CellClient* pClient) = 0;
private:

};


#endif // !_I_NET_EVENT_HPP_
