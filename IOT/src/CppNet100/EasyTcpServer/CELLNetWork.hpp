#ifndef _CELL_NET_WORK_HPP_
#define _CELL_NET_WORK_HPP_

#include "CELL.hpp"

class CELLNetWork
{
private:
	CELLNetWork()
	{
#ifdef _WIN32
		//启动windows socket 2.x环境
		WORD ver = MAKEWORD(2, 2);
		WSADATA data;
		WSAStartup(ver, &data);
#endif
		//
#ifndef _WIN32
		//if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		//	return (1);
		//网络通讯某一端（s或c）断开，会触发该信号，该信号默认是终止进程
		//忽略异常信号，默认情况会导致进程终止
		signal(SIGPIPE, SIG_IGN);
#endif
	}
	~CELLNetWork()
	{
#ifdef _WIN32
		//清除windows socket环境
		WSACleanup();
#endif
	}
public:
	static void Init()
	{
		static CELLNetWork obj;
	}
private:

};
#endif // !_CELL_NET_WORK_HPP_
