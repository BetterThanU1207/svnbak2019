﻿#include "EasyTcpServer.hpp"
#include <thread>

//bool g_bRun = true;
//void cmdThread()
//{
//	while (true)
//	{
//		// 3 输入请求
//		char cmdBuf[256] = {};
//		scanf_s("%s", cmdBuf, 256);
//		// 4 处理请求
//		if (0 == strcmp(cmdBuf, "exit"))
//		{
//			g_bRun = false;
//			printf("退出cmdThread线程。\n");
//			break;
//		}
//		else {
//			printf("不支持的命令。\n");
//		}
//	}
//}

class MyServer : public EasyTcpServer
{
public:
	//只会被一个线程触发 安全
	virtual void OnNetJoin(CellClient* pClient)
	{
		EasyTcpServer::OnNetJoin(pClient);
	}
	//cellServer 4 多个线程触发 不安全
	//如果只开启1个cellServer就是安全的
	virtual void OnNetLeave(CellClient* pClient)
	{
		EasyTcpServer::OnNetLeave(pClient);
	}
	//cellServer 4 多个线程触发 不安全
	//如果只开启1个cellServer就是安全的
	virtual void OnNetMsg(CellServer* pCellServer, CellClient* pClient, netmsg_DataHeader* header)
	{
		EasyTcpServer::OnNetMsg(pCellServer, pClient, header);
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			pClient->resetDTHeart();
			netmsg_Login* login = (netmsg_Login*)header;
			//printf("recv <Socket=%d> msgType：CMD_LOGIN, dataLen：%d,userName=%s PassWord=%s\n", cSock, login->dataLength, login->userName, login->PassWord);
			//忽略判断用户密码是否正确的过程
			netmsg_LoginR ret;
			if (SOCKET_ERROR == pClient->SendData(&ret))
			{
				//发送缓冲区满了，消息没发出去
				printf("recv <Socket=%d> msgType：Send Full\n", pClient->sockfd());
			}
			//先不使用任务系统
			//netmsg_LoginR* ret = new netmsg_LoginR();
			//pCellServer->addSendTask(pClient, ret);
		}
		break;
		case CMD_LOGOUT:
		{
			netmsg_Logout* logout = (netmsg_Logout*)header;
			//printf("recv <Socket=%d> msgType：CMD_LOGOUT, dataLen：%d,userName=%s \n", cSock, logout->dataLength, logout->userName);
			//忽略判断用户密码是否正确的过程
			//netmsg_LogoutR ret;
			//SendData(cSock, &ret);
		}
		break;
		case CMD_C2S_HEART:
		{
			pClient->resetDTHeart();
			netmsg_s2c_Heart ret;
			pClient->SendData(&ret);
		}
		break;
		default:
		{
			printf("recv <socket=%d> undefine msgType,dataLen：%d\n", pClient->sockfd(), header->dataLength);
		}
		break;
		}
	}
private:

};

int main()
{
	MyServer server;
	server.InitSocket();
	server.BindPort(nullptr, 4567);
	server.ListenPort(5);
	server.Start(4);
	//启动线程
	//std::thread t1(cmdThread);
	//t1.detach();//与主线程分离
	while (true)//server.isRun()
	{
		// 3 输入请求
		char cmdBuf[256] = {};
		scanf_s("%s", cmdBuf, 256);
		// 4 处理请求
		if (0 == strcmp(cmdBuf, "exit"))
		{
			server.CloseSocket();
			break;
		}
		else {
			printf("undefine cmd\n");
		}
	}

	printf("exit.\n");
#ifdef _WIN32
	while (true)
		Sleep(10);
#endif
	return 0;
}
