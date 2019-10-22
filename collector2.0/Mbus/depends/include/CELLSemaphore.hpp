#ifndef _CELL_SEMAPORE_HPP_
#define _CELL_SEMAPORE_HPP_

#include <chrono>
#include <thread>
//条件变量
#include <condition_variable>

// 信号量
class CELLSemaphore
{
public:
	void wait()
	{
		std::unique_lock<std::mutex> lock(_mutex);
		if (--_wait < 0)
		{
			//阻塞等待
			_cv.wait(lock, [this]()->bool {
				return _wakeup > 0;
				});
			--_wakeup;
		}		
	}
	void wakeup()
	{
		std::unique_lock<std::mutex> lock(_mutex);
		if (++_wait <= 0)
		{
			++_wakeup;
			_cv.notify_one();
		}
	}
private:
	std::mutex _mutex;
	//阻塞等待-条件变量
	std::condition_variable _cv;
	//等待计数
	int _wait = 0;
	//唤醒计数
	int _wakeup = 0;
};
#endif // !_CELL_SEMAPORE_HPP_

//虚假唤醒