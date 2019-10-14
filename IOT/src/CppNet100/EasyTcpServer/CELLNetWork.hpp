#ifndef _CELL_NET_WORK_HPP_
#define _CELL_NET_WORK_HPP_

#include "CELL.hpp"

class CELLNetWork
{
private:
	CELLNetWork()
	{
#ifdef _WIN32
		//����windows socket 2.x����
		WORD ver = MAKEWORD(2, 2);
		WSADATA data;
		WSAStartup(ver, &data);
#endif
		//
#ifndef _WIN32
		//if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		//	return (1);
		//����ͨѶĳһ�ˣ�s��c���Ͽ����ᴥ�����źţ����ź�Ĭ������ֹ����
		//�����쳣�źţ�Ĭ������ᵼ�½�����ֹ
		signal(SIGPIPE, SIG_IGN);
#endif
	}
	~CELLNetWork()
	{
#ifdef _WIN32
		//���windows socket����
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
