#include "EasyTcpServer.hpp"
#include <thread>

#include "RawDataDeal.h"
class MyServer : public EasyTcpServer
{
public:
	MyServer()
	{
		ClearClients();
	}
	~MyServer()
	{
		ClearClients();
	}
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
		//引发内存异常
		for (auto iter = _clients.begin(); iter != _clients.end();)
		{
			if (iter->second == pClient)
			{
				//delete iter->second;//后erase
				auto iterOld = iter;
				iter++;
				_clients.erase(iterOld);
				continue;
			}	
			iter++;
		}
	}
	//cellServer 4 多个线程触发 不安全
	//如果只开启1个cellServer就是安全的
	virtual void OnNetMsg(CellServer* pCellServer, CellClient* pClient, char* header, int len)
	{
		EasyTcpServer::OnNetMsg(pCellServer, pClient, header, len);
		//处理消息并返回相关（客户端ID，要发送数据客户端ID，发送数据）数据
		RawDataDeal data((const unsigned char*)header, len);
		std::string id = "";
		std::string sendId = "";
		std::string sendData = "sever";
		if (data.getData(id, sendId, sendData))
		{
			//记录在线客户端
			_clients[id] = pClient;
			//发送给指定客户端消息
			unsigned char buf;
			for (size_t i = 0; i < sendData.length(); )
			{				
				int func = strtol(sendData.substr(i, 2).c_str(), nullptr, 16);
				buf = toascii(func);
				auto iter = _clients.find(sendId);
				if (iter != _clients.end())
				{
					iter->second->SendData((char*)& buf, 1);
				}
				i += 2;
			}
			CELLLog::Info(sendData.c_str());
		}		
	}
private:
	void ClearClients()
	{
		for (auto iter : _clients)
		{
			delete iter.second;
		}
		_clients.clear();
	}
private:
	std::map<std::string, CellClient*> _clients;
};

int main()
{
	time_t nowtime;
	nowtime = time(NULL); //获取日历时间   
	char tmp[64];
	strftime(tmp, sizeof(tmp), "%Y-%m-%d serverLog.txt", localtime(&nowtime));	
	CELLLog::Instance().setLogPath(tmp, "a");

	MyServer server;
	server.InitSocket();
	server.BindPort(nullptr, 5010);
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
			CELLLog::Info("undefine cmd\n");
		}
	}

	CELLLog::Info("exit.\n");
#ifdef _WIN32
	while (true)
		Sleep(10);
#endif
	return 0;
}
