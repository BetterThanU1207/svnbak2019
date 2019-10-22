#include "EasyTcpServer.hpp"
#include <thread>
#include "CELLMsgStream.hpp"
#include "DataFromCollector.h"
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
	virtual void OnNetMsg(CellServer* pCellServer, CellClient* pClient, char* header, int len)
	{
		EasyTcpServer::OnNetMsg(pCellServer, pClient, header, len);
		DataFromCollector data;
		std::vector<std::string> str = data.dataResult((const unsigned char*)header, len);
		CELLLog::Info(str.at(2).c_str());
		//CELLRecvMsgStream r(header, len);
		//auto n1 = r.ReadInt8();
		//auto n2 = r.ReadInt16();
		//auto n3 = r.ReadInt32();
		//auto n4 = r.ReadFloat();
		//auto n5 = r.ReadDouble();
		//uint32_t n = 0;
		//r.onlyRead(n);
		//char* name = {};
		//auto n6 = r.ReadArray(name, n);
		//char pw[32] = {};
		//auto n7 = r.ReadArray(pw, 32);
		//int ata[10] = {};
		//auto n8 = r.ReadArray(ata, 10);
		///
		CELLSendMsgStream s(128);
		//s.WriteInt8(1);
		//s.WriteInt16(2);
		//s.WriteInt32(3);
		//s.WriteFloat(4.5f);
		//s.WriteDouble(6.7);

		s.WriteString("sever");
		//char a[] = "ahah";
		//s.WriteArray(a, strlen(a));
		//int b[] = { 1,2,3,4,5 };
		//s.WriteArray(b, 5);
		s.finsh();
		pClient->SendData(s.data(), s.length());
	}
private:

};

int main()
{
	CELLLog::Instance().setLogPath("serverLog.txt", "w");
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
