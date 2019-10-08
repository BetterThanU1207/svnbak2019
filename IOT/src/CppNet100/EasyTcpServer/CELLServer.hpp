#ifndef _CELL_SERVER_HPP
#define _CELL_SERVER_HPP

#include <vector>
#include<map>

#include "CELL.hpp"
#include "INetEvent.hpp"
#include "CELLClient.hpp"

//网络消息接收服务类
class CellServer
{
public:
	CellServer(SOCKET sock = INVALID_SOCKET)
	{
		_sock = sock;
		_pNetEvent = nullptr;
	}
	~CellServer()
	{
		CloseSocket();
		_sock = INVALID_SOCKET;
	}

	void setEventObj(INetEvent* event)
	{
		_pNetEvent = event;
	}

	//判断是否工作中
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}
	//处理网络消息
	//备份客户socket fd_set
	fd_set _fdRead_bak;
	//客户列表是否有变化
	bool _clients_change;
	SOCKET _maxSock;
	void OnRun()
	{
		_clients_change = true;
		while (isRun())
		{
			if (!_clientsBuff.empty())
			{
				//从缓冲队列里取出客户数据
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuff)
				{
					_clients[pClient->sockfd()] = pClient;
				}
				_clientsBuff.clear();
				_clients_change = true;
			}
			//如果没有需要处理的客户端，就跳过
			if (_clients.empty())
			{
				//休眠一毫秒
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}
			//伯克利套接字 BSDsocket
			fd_set fdRead;//描述符（socket）集合
			//fd_set fdWrite;
			//fd_set fdExp;
			//集合计数清零
			FD_ZERO(&fdRead);
			//FD_ZERO(&fdWrite);
			//FD_ZERO(&fdExp);
			//将服务端socket加入集合
			//FD_SET(_sock, &fdRead); 不应该两个地方对同一个socket查询
			//FD_SET(_sock, &fdWrite);
			//FD_SET(_sock, &fdExp);

			if (_clients_change)
			{
				_clients_change = false;
				//将描述符（socket）加入集合
				_maxSock = _clients.begin()->second->sockfd();
				for (auto iter : _clients)
				{
					//将客户端socket加入集合
					FD_SET(iter.second->sockfd(), &fdRead);
					if (_maxSock < iter.second->sockfd())
					{
						_maxSock = iter.second->sockfd();
					}
				}
				memcpy(&_fdRead_bak, &fdRead, sizeof(fd_set));
			}
			else
			{
				memcpy(&fdRead, &_fdRead_bak, sizeof(fd_set));
			}

			//nfds 是一个整数值 是指fd_set集合中所有描述符（socket）的范围，而不是数量，
			//既是所有文件描述符最大值+1 在windows中这个参数可以写0
			//select最后一个参数是null，是阻塞模式（有数据可操作的时候才返回），纯接收数据的服务可以接受
			//timeval t = { 1, 0 };//查询时间为1 最大查询时间为1并非等待1s 非阻塞网络模型  综合性网络程序\
			//只收数据，不需要主动查询实例
			int ret = select(_maxSock + 1, &fdRead, nullptr, nullptr, nullptr);
			if (ret < 0)
			{
				printf("select任务结束。\n");
				CloseSocket();
				return;
			}
			else if (ret == 0)
			{
				continue;
			}

#ifdef _WIN32
			for (int n = 0; n < fdRead.fd_count; n++)
			{
				auto iter = _clients.find(fdRead.fd_array[n]);
				if (iter != _clients.end())
				{
					if (-1 == RecvData(iter->second))
					{
						if (_pNetEvent)
							_pNetEvent->OnNetLeave(iter->second);
						_clients_change = true;
						_clients.erase(iter->first);
					}
				}
				else {
					printf("error. if (iter != _clients.end())\n");
				}

			}
#else
			std::vector<CellClient*> temp;
			for (auto iter : _clients)
			{
				if (FD_ISSET(iter.second->sockfd(), &fdRead))
				{
					if (-1 == RecvData(iter.second))
					{
						if (_pNetEvent)
							_pNetEvent->OnNetLeave(iter.second);
						_clients_change = false;
						temp.push_back(iter.second);
					}
				}
			}
			for (auto pClient : temp)
			{
				_clients.erase(pClient->sockfd());
				delete pClient;
			}
#endif
		}
	}

	//缓冲区
	//接收数据 处理粘包 拆分包
	int RecvData(CellClient* pClient)
	{
		char* szRecv = pClient->msgBuf() + pClient->getLastPos();
		// 5 接收数据
		int nLen = (int)recv(pClient->sockfd(), szRecv, RECV_BUFF_SIZE - pClient->getLastPos(), 0);
		_pNetEvent->OnNetRecv(pClient);
		if (nLen <= 0)
		{
			//printf("客户端<Socket=%d>已退出，任务结束。\n", pClient->sockfd());
			return -1;
		}
		//将收取的数据拷贝到消息缓冲区
		//memcpy(pClient->msgBuf() + pClient->getLastPos(), _szRecv, nLen);
		//消息缓冲区的数据尾部位置后移
		pClient->setLastPos(pClient->getLastPos() + nLen);
		//判断消息缓冲区的数据长度是否大于消息头netmsg_DataHeader长度		
		//积压消息够多有可能会阻塞
		while (pClient->getLastPos() >= sizeof(netmsg_DataHeader))//循环解决粘包
		{
			//这是就可以知道当前消息的长度
			netmsg_DataHeader* header = (netmsg_DataHeader*)pClient->msgBuf();
			//判断消息缓冲区的数据长度大于消息长度		
			if (pClient->getLastPos() >= header->dataLength)//判断解决少包   也可可能两者相等，那nSize=0
			{
				//消息缓冲区剩余未处理数据的长度   需要提前保存下来
				int nSize = pClient->getLastPos() - header->dataLength;//从接受缓冲区多取过来的一部分
				//处理网络消息
				OnNetMsg(pClient, header);//header被处理过后其中header被强制转换和位移已经改变
				//将消息缓冲区剩余未处理数据前移
				memcpy(pClient->msgBuf(), pClient->msgBuf() + header->dataLength, nSize);
				//消息缓冲区的数据尾部位置前移
				pClient->setLastPos(nSize);
			}
			else
			{
				//消息缓冲区剩余数据不够一条完整消息
				break;
			}
		}
		return 0;
	}
	//响应网络消息
	virtual void OnNetMsg(CellClient* pClient, netmsg_DataHeader* header)
	{
		_pNetEvent->OnNetMsg(this, pClient, header);
	}
	//关闭socket
	void CloseSocket()
	{
		if (INVALID_SOCKET != _sock)
		{
#ifdef _WIN32
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				closesocket(_clients[n]->sockfd());
				delete _clients[n];//非安全删除，应判空
			}
			//closesocket 关闭套接字
			closesocket(_sock);
#else
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				close(_clients[n]->sockfd());
			}
			//closesocket 关闭套接字
			close(_sock);
#endif
		}
		_clients.clear();
	}

	void addClient(CellClient* pClient)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		//_mutex.lock();
		_clientsBuff.push_back(pClient);
		//_mutex.unlock();
	}

	void Start()
	{
		_thread = std::thread(std::mem_fn(&CellServer::OnRun), this);
		_taskServer.Start();
	}

	size_t getClientCount()
	{
		return _clients.size() + _clientsBuff.size();
	}

	void addSendTask(CellClient* pClient, netmsg_DataHeader* header)
	{
		//拉曼达函数
		_taskServer.addTask( [pClient, header] () {
			pClient->SendData(header);
			delete header;
		});
	}
private:
	SOCKET _sock;
	//正式客户队列
	std::map<SOCKET, CellClient*> _clients;
	//缓冲客户队列
	std::vector<CellClient*> _clientsBuff;
	//缓冲队列的锁
	std::mutex _mutex;
	std::thread _thread;
	//网络事件对象
	INetEvent* _pNetEvent;
	//
	CellTaskServer _taskServer;
};


#endif // !_CELL_SERVER_HPP
