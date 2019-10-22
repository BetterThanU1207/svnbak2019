#ifndef _CELL_MSG_STREAM_HPP_
#define _CELL_MSG_STREAM_HPP_

#include "CELLStream.hpp"

//消息数据字节流BYTE
class CELLRecvMsgStream : public CELLStream
{
public:
	CELLRecvMsgStream(char* header, int len)
		: CELLStream((char*)header, len)
	{
		//push(strlen(header));
	}

public:
	
};

class CELLSendMsgStream : public CELLStream
{
public:
	CELLSendMsgStream(char* pData, int nSize, bool bDelete = false)
		: CELLStream(pData, nSize, bDelete)
	{
		//预先占领消息长度所需空间
		Write<uint16_t>(0);
	}

	CELLSendMsgStream(int nSize = 1024)
		: CELLStream(nSize)
	{
	}

	bool WriteString(const char* str, int len)
	{
		return WriteArray(str, len);
	}
	bool WriteString(const char* str)
	{
		return WriteArray(str, strlen(str));
	}
	bool WriteString(std::string& str)
	{
		return WriteArray(str.c_str(), str.length());
	}

	void finsh()
	{
		int pos = getWritePos();
		setWritePos(0);
		Write<uint16_t>(pos);
		setWritePos(pos);
	}
};

#endif // !_CELL_MSG_STREAM_HPP_
