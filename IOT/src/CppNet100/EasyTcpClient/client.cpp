#include <stdio.h>
#include <thread>
#include <atomic>
#include "EasyTcpClient.hpp"
#include "CELLTimestamp.hpp"
#include "CELLMsgStream.hpp"

class MyClient : public EasyTcpClient
{
public:
	//响应网络消息
	virtual void OnNetMsg(netmsg_DataHeader* header)
	{
		// 6 处理请求		
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			netmsg_LoginR* login = (netmsg_LoginR*)header;
			//CELLLog::Info("<socket=%d>recv server msg:CMD_LOGIN_RESULT msg:%d \n", (int)_sock, login->result);
		}
		break;
		case CMD_LOGOUT_RESULT:
		{
			netmsg_LogoutR* logout = (netmsg_LogoutR*)header;
			//CELLLog::Info("<socket=%d>recv server msg:CMD_LOGOUT_RESULT msg:%d \n", (int)_sock, logout->result);
		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			netmsg_NewUserJoin* userJoin = (netmsg_NewUserJoin*)header;
			//CELLLog::Info("<socket=%d>recv server msg:CMD_NEW_USER_JOIN msg:%d \n", (int)_sock, userJoin->socketID);
		}
		break;
		case CMD_ERROR:
		{
			CELLLog::Info("<socket=%d>recv server msg:CMD_ERROR，dataLength:%d \n", (int)_pClient->sockfd(), header->dataLength);
		}
		break;
		default:
		{
			CELLRecvMsgStream r(header);
			//auto n1 = r.ReadInt8();
			//auto n2 = r.ReadInt16();
			//auto n3 = r.ReadInt32();
			//auto n4 = r.ReadFloat();
			//auto n5 = r.ReadDouble();
			//uint32_t n = 0;
			//r.onlyRead(n);
			char name[32] = {};
			auto n6 = r.ReadArray(name, 32);
			//char pw[32] = {};
			//auto n7 = r.ReadArray(pw, 32);
			//int ata[10] = {};
			//auto n8 = r.ReadArray(ata, 10);
			CELLLog::Info("error, <socket=%d>recv undefined msgType.\n", (int)_pClient->sockfd());
		}
		break;
		}
	}
private:

};

bool g_bRun = true;
void cmdThread( /*EasyTcpClient* client*/)
{
	while (true)
	{
		// 3 输入请求
		char cmdBuf[256] = {};
		scanf_s("%s", cmdBuf, 256);
		// 4 处理请求
		if (0 == strcmp(cmdBuf, "exit"))
		{
			g_bRun = false;
			//client->CloseSocket();
			CELLLog::Info("exit cmdThread.\n");
			break;
		}
		//else if (0 == strcmp(cmdBuf, "login"))
		//{
		//	netmsg_Login login;
		//	strcpy_s(login.userName, "lyd");
		//	strcpy_s(login.passWord, "lydmm");
		//	// 5 向服务器发送请求
		//	client->SendData(&login);
		//}
		//else if (0 == strcmp(cmdBuf, "logout"))
		//{
		//	Logout logout;
		//	strcpy_s(logout.userName, "lyd");
		//	// 5 向服务器发送请求
		//	client->SendData(&logout); 
		//}
		else
		{
			CELLLog::Info("undefined cmd, please try again.\n");
		}
	}	
}

//客户端数量
const int cCount = 1;//windows默认客户端最大个数，减去一个服务端，超出了则不会传输数据
//线程数量
const int tCount = 1;
//需要用指针，不然栈内存会爆掉
//客户端数组
EasyTcpClient* client[cCount];//声明多个对象就可以连接多个服务器
std::atomic_int sendCount = 0;
std::atomic_int readyCount = 0;

void recvThread(int begin, int end)
{
	//CELLTimestamp t;
	while (g_bRun)
	{
		for (int n = begin; n < end; n++)
		{
			//if (t.getElapsedSecond() > 3.0 && n == begin)
			//	continue;
			client[n]->OnRun();
		}
	}
}

void sendThread(int id)
{	
	CELLLog::Info("thread<%d> start\n", id);
	//4个线程 ID 1~4
	int c = cCount / tCount;
	int begin = (id - 1) * c;
	int end = id * c;

	for (int n = begin; n < end; n++)
	{
		client[n] = new MyClient();
	}
	for (int n = begin; n < end; n++)
	{		
		client[n]->ConnectServer("127.0.0.1", 4567);
	}

	CELLLog::Info("thread<%d>,Connect<begin=%d, end=%d>\n", id, begin, end);

	readyCount++;
	while (readyCount < tCount)
	{
		//休眠  等待其他线程准备好发送数据 并发
		CELLThread::Sleep(10);
	}
	//
	std::thread t1(recvThread, begin, end);
	t1.detach();
	//组织发送数据
	netmsg_Login login[1];
	for (int n = 0; n < 1; n++)
	{
		strcpy_s(login[n].userName, "lyd");
		strcpy_s(login[n].passWord, "lydmm");
	}
	const int nLen = sizeof(login);

	CELLSendMsgStream s(128);
	s.setNetCmd(100);
	s.WriteString("client");
	s.finsh();

	while (g_bRun)
	{
		for (int n = begin; n < end; n++)
		{
			if (SOCKET_ERROR != client[n]->SendData(s.data(), s.length()))
			{
				sendCount++;
			}			
		}
		//休眠  等待其他线程准备好发送数据 并发
		CELLThread::Sleep(100);
	}

	for (int n = begin; n < end; n++)
	{
		client[n]->CloseSocket();
		delete client[n];
	}
	CELLLog::Info("thread<%d> exit\n", id);
}

int main()
{
	CELLLog::Instance().setLogPath("clientLog.txt", "w");
	//启动UI线程
	std::thread t1(cmdThread);
	t1.detach();//与主线程分离	

	//启动发送线程
	for (int i = 0; i < tCount; i++)
	{
		std::thread t1(sendThread, i + 1);
		t1.detach();
	}

	CELLTimestamp tTime;

	while (g_bRun)
	{
		auto t = tTime.getElapsedSecond();
		if (t >= 1.0)
		{
			CELLLog::Info("thread<%d>, clients<%d>,time<%lf>,send<%d>\n", tCount, cCount, t, (int)(sendCount / t));
			tTime.update();
			sendCount = 0;
		}
		Sleep(1);
	}
	
	CELLLog::Info("exit，finshed。\n");
	getchar();
	return 0;
}