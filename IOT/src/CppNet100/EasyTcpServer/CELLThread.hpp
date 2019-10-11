#ifndef _CELL_THREAD_HPP_
#define _CELL_THREAD_HPP_

#include <functional>//mem_fun 安全转换
#include "CELLSemaphore.hpp"

class CELLThread
{
private:
	typedef std::function<void(CELLThread*)> EventCall;
public:
	//启动工作线程 参数是回调函数
	void Start(EventCall onCreate = nullptr, EventCall onRun = nullptr, EventCall onDestory = nullptr)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (!_isRun)
		{
			_isRun = true;

			if (onCreate)
			{
				_onCreate = onCreate;
			}
			if (onRun)
			{
				_onRun = onRun;
			}
			if (onDestory)
			{
				_onDestory = onDestory;
			}
			//线程 先赋值再启动线程
			std::thread t(std::mem_fn(&CELLThread::OnWork), this);
			t.detach();
		}
	}
	//关闭线程
	void Close()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_isRun)
		{
			_isRun = false;
			_sem.wait();
		}
	}
	//在工作函数中退出 
	//不需要使用信号量来阻塞等待
	//如果使用会阻塞
	void Exit()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_isRun)
		{
			_isRun = false;
		}
	}
	//线程是否启动运行状态
	bool isRun()
	{
		return _isRun;
	}
protected:
	//线程的运行时的工作函数
	void OnWork()
	{
		if (_onCreate)
		{
			_onCreate(this);
		}
		if (_onRun)
		{
			_onRun(this);
		}
		if (_onDestory)
		{
			_onDestory(this);
		}
		_sem.wakeup();
	}

private:
	//
	EventCall _onCreate;
	//
	EventCall _onRun;
	//
	EventCall _onDestory;	
	//不同线程改变数据时需要加锁
	std::mutex _mutex;
	//控制线程的终止、退出
	CELLSemaphore _sem;
	//线程是否启动运行中
	bool _isRun = false;
};


#endif // !_CELL_THREAD_HPP_
