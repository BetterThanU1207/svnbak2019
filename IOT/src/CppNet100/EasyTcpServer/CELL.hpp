#ifndef _CELL_HPP_
#define _CELL_HPP_

//SOCKET相关跨平台头文件
#ifdef _WIN32
		#define FD_SETSIZE			2506//突破windows下select64个的限制
		#define WIN32_LEAN_AND_MEAN	//避免引用早期的windows库
		#define _WINSOCK_DEPRECATED_NO_WARNINGS	//避免不能使用socket的旧函数出现错误
		#include <WS2tcpip.h>	//socket的新函数头文件
		#include <windows.h>
		#include <WinSock2.h>
		#pragma comment(lib, "ws2_32.lib")
#else
		#include <unistd.h>//uni std unix系统下的标准库
		#include<arpa/inet.h>
		#include <string.h>

		#define  SOCKET int
		#define  INVALID_SOCKET		(SOCKET)(~0)
		#define  SOCKET_ERRROR						(-1)
#endif 

//自定义
#include "MessageHeader.hpp"
#include "CELLTimestamp.hpp"
#include "CELLTask.hpp"
//
#include <stdio.h>

//缓冲区最小单元大小
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240
#define SEND_BUFF_SIZE RECV_BUFF_SIZE
#endif

#endif // !_CELL_HPP_