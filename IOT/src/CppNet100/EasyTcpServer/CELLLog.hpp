#ifndef _CELL_LOG_HPP_
#define _CELL_LOG_HPP_

#include "CELL.hpp"
#include <ctime>
#pragma warning( disable : 4996 )	//���fopen_s�� gmtime_s�Ȱ�ȫ��������
class CELLLog
{
public:
	CELLLog()
	{
		_taskServer.Start();
	}
	~CELLLog()
	{
		_taskServer.Close();
		if (_logFile)
		{
			Info("~CELLLog fclose(_logFile)\n");
			fclose(_logFile);
			_logFile = nullptr;
		}
	}

public:
	static CELLLog& Instance()
	{
		static CELLLog sLog;
		return sLog;
	}

	void setLogPath(const char* logPath, const char* mode)
	{
		if (_logFile)
		{
			Info("CELLLog::setLogPath _logFile  != nullptr\n");
			fclose(_logFile);
			_logFile = nullptr;
		}
		//fopen_s(&_logFile, logPath, mode);//���������в��ܹ����ļ�
		_logFile = fopen(logPath, mode);
		if (_logFile)
		{
			Info("CELLLog::setLogPath success, <%s, %s>\n", logPath, mode);
		}
		else
		{
			Info("CELLLog::setLogPath failed, <%s, %s>\n", logPath, mode);
		}
	}

	//Debug
	//Warning
	//Error
	//Info
	static void Info(const char* pStr)
	{
		CELLLog* pLog = &Instance();
		//������һ���߳�
		pLog->_taskServer.addTask([pLog, pStr]() {
			if (pLog->_logFile)
			{
				auto t = system_clock::now();
				auto tNow = system_clock::to_time_t(t);
				//fprintf(pLog->_logFile, "%s", ctime(&tNow));
				std::tm* now = std::gmtime(&tNow);
				fprintf(pLog->_logFile, "[%d-%d-%d %d:%d:%d", now->tm_year + 1900, now->tm_mon, now->tm_mday,
					now->tm_hour + 8, now->tm_min, now->tm_sec);
				fprintf(pLog->_logFile, "%s", pStr);
				fflush(pLog->_logFile);//ʵʱд���ļ�
			}
		}
		);
		printf("%s", pStr);
	}
	template<typename ... Args>
	static void Info(const char* pFormat,  Args ... args)
	{
		CELLLog* pLog = &Instance();
		//������һ���߳�
		pLog->_taskServer.addTask( [pLog, pFormat, args...] () {
				if (pLog->_logFile)
				{
					auto t = system_clock::now();
					auto tNow = system_clock::to_time_t(t);
					//fprintf(pLog->_logFile, "%s", ctime(&tNow));
					std::tm* now = std::gmtime(&tNow);
					fprintf(pLog->_logFile, "[%d-%d-%d %d:%d:%d", now->tm_year + 1900, now->tm_mon, now->tm_mday,
						now->tm_hour + 8, now->tm_min, now->tm_sec);
					fprintf(pLog->_logFile, pFormat, args...);
					fflush(pLog->_logFile);//ʵʱд���ļ�
				}
			}
		);
		
		printf(pFormat, args ...);
	}

private:
	FILE* _logFile = nullptr;
	CellTaskServer _taskServer;
};


#endif // !_CELL_LOG_HPP_
