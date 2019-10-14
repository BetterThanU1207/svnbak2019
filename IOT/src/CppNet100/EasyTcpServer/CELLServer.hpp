#ifndef _CELL_SERVER_HPP_
#define _CELL_SERVER_HPP_

#include <vector>
#include<map>

#include "CELL.hpp"
#include "INetEvent.hpp"
#include "CELLClient.hpp"
#include "CELLSemaphore.hpp"

//网络消息接收服务类
class CellServer
{
public:
	CellServer(int id)
	{
		_id = id;
		_pNetEvent = nullptr;
		_taskServer.serverId = id;
	}
	~CellServer()
	{
		printf("CellServer%d.~CellServer exit begin 1\n", _id);
		CloseSocket();		
		printf("CellServer%d.~CellServer exit end 1\n", _id);
	}

	void setEventObj(INetEvent* event)
	{
		_pNetEvent = event;
	}

	//关闭socket
	void CloseSocket()
	{
		printf("CellServer%d.CloseSocket exit begin\n", _id);
		_taskServer.Close();
		_thread.Close();
		printf("CellServer%d.CloseSocket exit end\n", _id);
	}

	//判断是否工作中
	//bool isRun()
	//{
	//	return _sock != INVALID_SOCKET;
	//}
	//处理网络消息
	void OnRun(CELLThread* pThread)
	{
		while (pThread->isRun())
		{
			if (!_clientsBuff.empty())
			{
				//从缓冲队列里取出客户数据
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuff)
				{
					_clients[pClient->sockfd()] = pClient;
					pClient->serverId = _id;
					if (_pNetEvent)
					{
						_pNetEvent->OnNetJoin(pClient);
					}									
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
				//旧的时间戳
				_oldTime = CELLTime::getNowInMilliSec();
				continue;
			}
			//伯克利套接字 BSDsocket
			fd_set fdRead;//描述符（socket）集合
			fd_set fdWrite;
			//fd_set fdExc;
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
			memcpy(&fdWrite, &_fdRead_bak, sizeof(fd_set));//可以根据业务情况，只把可写的放入
			//memcpy(&fdExc, &_fdRead_bak, sizeof(fd_set));
			//nfds 是一个整数值 是指fd_set集合中所有描述符（socket）的范围，而不是数量，
			//既是所有文件描述符最大值+1 在windows中这个参数可以写0
			//select最后一个参数是null，是阻塞模式（有数据可操作的时候才返回），纯接收数据的服务可以接受
			//timeval t = { 1, 0 };//查询时间为1 最大查询时间为1并非等待1s 非阻塞网络模型  综合性网络程序\
			//只收数据，不需要主动查询实例
			timeval t{ 0,1 };
			int ret = select(_maxSock + 1, &fdRead, &fdWrite, nullptr, &t);
			if (ret < 0)
			{
				printf("CellServer%d.OnRun.select Error exit\n", _id);
				pThread->Exit();
				break;
			}
			//else if (ret == 0)
			//{
			//	continue;
			//}
			ReadData(fdRead);
			WriteData(fdWrite);
			//WriteData(fdExc);//也可用ReadData
			//printf("CellServer%d.OnRun select fdRead=%d\n", _id, fdRead.fd_count);
			//printf("CellServer%d.OnRun select fdWrite=%d\n", _id, fdWrite.fd_count);
			//if (fdExc.fd_count > 0)
			//{
			//	printf("###CellServer%d.OnRun select fdExc=%d\n", _id, fdExc.fd_count);
			//}
			
			CheckTime();
		}
		printf("CellServer%d.OnRun exit\n", _id);
	}
	
	void CheckTime()
	{
		//当前时间戳
		auto nowTime = CELLTime::getNowInMilliSec();
		auto dt = nowTime - _oldTime;
		_oldTime = nowTime;

		for (auto iter = _clients.begin(); iter != _clients.end();)
		{
			//心跳检测
			if (iter->second->checkHeart(dt))
			{
				if (_pNetEvent)
				{
					_pNetEvent->OnNetLeave(iter->second);
				}
				_clients_change = true;
				delete iter->second;
				auto iterOld = iter;
				iter++;
				_clients.erase(iterOld);
				continue;
			}
			//定时发送检测
			//iter->second->checkSend(dt);
			iter++;
		}
	}
	
	void OnClientLeave(CellClient* pClient)
	{
		if (_pNetEvent)
			_pNetEvent->OnNetLeave(pClient);
		_clients_change = true;
		delete pClient;
	}

	void WriteData(fd_set& fdWrite)
	{
#ifdef _WIN32
		for (int n = 0; n < fdWrite.fd_count; n++)
		{
			auto iter = _clients.find(fdWrite.fd_array[n]);
			if (iter != _clients.end())
			{
				if (-1 == iter->second->SendDataReal())
				{
					OnClientLeave(iter->second);
					_clients.erase(iter);
				}
			}
		}
#else
		for (auto iter = _clients.begin(); iter != _clients.end(); )
		{
			if (FD_ISSET(iter->second->sockfd(), &fdWrite))
			{
				if (-1 == iter->second->SendDataReal())
				{
					OnClientLeave(iter->second);
					auto iterOld = iter;
					iter++;
					_clients.erase(iterOld);
					continue;
				}
			}
			iter++;
		}
#endif
	}
	
	void ReadData(fd_set& fdRead)
	{
#ifdef _WIN32
		for (int n = 0; n < fdRead.fd_count; n++)
		{
			auto iter = _clients.find(fdRead.fd_array[n]);
			if (iter != _clients.end())
			{
				if (-1 == RecvData(iter->second))
				{
					OnClientLeave(iter->second);
					_clients.erase(iter);
				}
			}
		}
#else
		for (auto iter = _clients.begin(); iter != _clients.end(); )
		{
			if (FD_ISSET(iter->second->sockfd(), &fdRead))
			{
				if (-1 == RecvData(iter->second))
				{
					OnClientLeave(iter->second);
					auto iterOld = iter;
					iter++;
					_clients.erase(iterOld);
					continue;
				}
			}
			iter++;
		}
#endif
	}

	//缓冲区
	//接收数据 处理粘包 拆分包
	int RecvData(CellClient* pClient)
	{
		int nLen = pClient->RecvData();		
		if (nLen <= 0)
		{
			return -1;
		}
		//触发<接收到网络数据>事件
		_pNetEvent->OnNetRecv(pClient);
		//循环 判断是否有消息需要处理
		while (pClient->hasMsg())//循环解决粘包
		{
			//处理网络消息
			OnNetMsg(pClient, pClient->front_msg());//header被处理过后其中header被强制转换和位移已经改变
			//移除消息队列（缓冲区）最前的一条数据
			pClient->pop_front_msg();
		}
		return 0;
	}
	//响应网络消息
	virtual void OnNetMsg(CellClient* pClient, netmsg_DataHeader* header)
	{
		_pNetEvent->OnNetMsg(this, pClient, header);
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
		_taskServer.Start();
		_thread.Start(
			//onCreate
			nullptr, 
			//onRun
			[this](CELLThread* pThread) {
					OnRun(pThread);
				},
			//OnDestroy
			[this](CELLThread* pThread) {
					ClearClients();
				}
		);		
	}

	size_t getClientCount()
	{
		return _clients.size() + _clientsBuff.size();
	}

	//void addSendTask(CellClient* pClient, netmsg_DataHeader* header)
	//{
	//	//拉曼达函数
	//	_taskServer.addTask( [pClient, header] () {
	//		pClient->SendData(header);
	//		delete header;
	//	});
	//}
private:
		void ClearClients()
		{
			for (auto iter : _clients)
			{
				delete iter.second;
			}
			_clients.clear();

			for (auto iterBuf : _clientsBuff)
			{
				delete iterBuf;
			}
			_clientsBuff.clear();
		}
private:
	//正式客户队列
	std::map<SOCKET, CellClient*> _clients;
	//缓冲客户队列
	std::vector<CellClient*> _clientsBuff;
	//缓冲队列的锁
	std::mutex _mutex;
	//网络事件对象
	INetEvent* _pNetEvent;
	//
	CellTaskServer _taskServer;
	//备份客户socket fd_set
	fd_set _fdRead_bak;
	//
	SOCKET _maxSock;
	//旧的时间戳
	time_t _oldTime = CELLTime::getNowInMilliSec();
	//
	CELLThread _thread;
	//
	int _id = -1;
	//客户列表是否有变化
	bool _clients_change;
};
#endif // !_CELL_SERVER_HPP
