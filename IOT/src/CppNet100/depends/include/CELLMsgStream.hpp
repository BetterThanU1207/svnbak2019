#ifndef _CELL_MSG_STREAM_HPP_
#define _CELL_MSG_STREAM_HPP_

#include "CELLStream.hpp"
#include "MessageHeader.hpp"

//消息数据字节流BYTE
class CELLRecvMsgStream : public CELLStream
{
public:
	CELLRecvMsgStream(netmsg_DataHeader* header)
		: CELLStream((char*)header, header->dataLength)
	{
		push(header->dataLength);
		//预读取消息长度
		ReadInt16();
		//预读取消息命令
		getNetCmd();
	}

	int getNetCmd()
	{
		uint16_t cmd = CMD_ERROR;
		Read<uint16_t>(cmd);
		return cmd;
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
		//预先占领消息长度所需空间
		Write<uint16_t>(0);
	}

	void setNetCmd(uint16_t cmd)
	{
		Write<uint16_t>(cmd);
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
