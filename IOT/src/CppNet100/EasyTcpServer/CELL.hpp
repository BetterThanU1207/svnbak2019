#ifndef _CELL_HPP_
#define _CELL_HPP_

//SOCKET��ؿ�ƽ̨ͷ�ļ�
#ifdef _WIN32
		#define FD_SETSIZE			2506//ͻ��windows��select64��������
		#define WIN32_LEAN_AND_MEAN	//�����������ڵ�windows��
		#define _WINSOCK_DEPRECATED_NO_WARNINGS	//���ⲻ��ʹ��socket�ľɺ������ִ���
		#include <WS2tcpip.h>	//socket���º���ͷ�ļ�
		#include <windows.h>
		#include <WinSock2.h>
		#pragma comment(lib, "ws2_32.lib")
#else
		#include <unistd.h>//uni std unixϵͳ�µı�׼��
		#include<arpa/inet.h>
		#include <string.h>

		#define  SOCKET int
		#define  INVALID_SOCKET		(SOCKET)(~0)
		#define  SOCKET_ERRROR						(-1)
#endif 

//�Զ���
#include "MessageHeader.hpp"
#include "CELLTimestamp.hpp"
#include "CELLTask.hpp"
//
#include <stdio.h>

//��������С��Ԫ��С
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240
#define SEND_BUFF_SIZE RECV_BUFF_SIZE
#endif

#endif // !_CELL_HPP_