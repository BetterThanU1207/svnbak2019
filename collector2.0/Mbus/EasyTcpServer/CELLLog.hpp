/*
文档说明：日志打印和文件存储
*/
#ifndef _CELL_LOG_HPP_
#define _CELL_LOG_HPP_

#include "CELL.hpp"
#include <ctime>
#pragma warning( disable : 4996 )	//解决fopen_s， gmtime_s等安全性问题检查
class CELLLog
{
private:
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
		//fopen_s(&_logFile, logPath, mode);//程序运行中不能够打开文件
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
		//放入另一个线程
		pLog->_taskServer.addTask([pLog, pStr]() {
			if (pLog->_logFile)
			{
				auto t = system_clock::now();
				auto tNow = system_clock::to_time_t(t);
				//fprintf(pLog->_logFile, "%s", ctime(&tNow));
				std::tm* now = std::gmtime(&tNow);
				fprintf(pLog->_logFile, "[%d-%d-%d %d:%d:%d]---", now->tm_year + 1900, now->tm_mon, now->tm_mday,
					now->tm_hour + 8, now->tm_min, now->tm_sec);
				fprintf(pLog->_logFile, "%s", pStr);
				fflush(pLog->_logFile);//实时写入文件
			}
		}
		);
		printf("%s", pStr);
	}
	template<typename ... Args>
	static void Info(const char* pFormat,  Args ... args)
	{
		CELLLog* pLog = &Instance();
		//放入另一个线程
		pLog->_taskServer.addTask( [pLog, pFormat, args...] () {
				if (pLog->_logFile)
				{
					auto t = system_clock::now();
					auto tNow = system_clock::to_time_t(t);
					//fprintf(pLog->_logFile, "%s", ctime(&tNow));
					std::tm* now = std::gmtime(&tNow);
					fprintf(pLog->_logFile, "[%d-%d-%d %d:%d:%d]---", now->tm_year + 1900, now->tm_mon, now->tm_mday,
						now->tm_hour + 8, now->tm_min, now->tm_sec);
					fprintf(pLog->_logFile, pFormat, args...);
					fflush(pLog->_logFile);//实时写入文件
				}
			}
		);
		
		printf(pFormat, args ...);
	}

	// Prints the contents of memory in hex and ascii.
	// Prints the memory between and including the
	// two "end1" and "end2" pointers.
	static void Show_Memory(const unsigned char* end1, const unsigned char* end2)
	{
		if (end2 >= end1)
		{
			CELLLog::Instance().Print_Memory(end1, end2 - end1);
		}
		else
		{
			CELLLog::Instance().Print_Memory(end2, end1 - end2 + 1);
		}
	}

private:
	void Print_Memory(const unsigned char* start, unsigned int length)
	{
		//create row, col, and i.  Set i to 0
		int row, col, i = 0;

		//iterate through the rows, which will be 16 bytes of memory wide
		for (row = 0; (i + 1) < length; row++)
		{
			//print hex representation
			for (col = 0; col < 16; col++)
			{
				//calculate the current index
				i = row * 16 + col;

				//divides a row of 16 into two columns of 8
				if (col == 8)
				{
					printf(" ");
				}

				//print the hex value if the current index is in range.
				if (i < length)
				{
					printf("%02X", start[i]);
				}
				//print a blank if the current index is past the end
				else
				{
					printf("  ");
				}

				//print a space to keep the values separate
				printf(" ");
			}

			//create a vertial seperator between hex and ascii representations
			//printf(" ");

			////print ascii representation
			//for (col = 0; col < 16; col++)
			//{
			//	//calculate the current index
			//	i = row * 16 + col;

			//	//divides a row of 16 into two coumns of 8
			//	if (col == 8)
			//	{
			//		printf("  ");
			//	}

			//	//print the value if it is in range
			//	if (i < length)
			//	{
			//		//print the ascii value if applicable
			//		if (start[i] > 0x20 && start[i] < 0x7F)  //A-Z
			//		{
			//			printf("%c", start[i]);
			//		}
			//		//print a period if the value is not printable
			//		else
			//		{
			//			printf(".");
			//		}
			//	}
			//	//nothing else to print, so break out of this for loop
			//	else
			//	{
			//		break;
			//	}
			//}

			//create a new row
			//printf("\n");
		}
	}	

private:
	FILE* _logFile = nullptr;
	CellTaskServer _taskServer;
};


#endif // !_CELL_LOG_HPP_
