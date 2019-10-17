/*
文档说明：网络消息类型
*/
#ifndef _MessageHeader_hpp_
#define _MessageHeader_hpp_

enum CMD
{
	CMD_DT_SET = 1,
	CMD_WRITE_IDS,
	CMD_UP_DATAS,
	CMD_READ_DATA,
	CMD_READ_PARAS,
	CMD_READ_IDS,
	CMD_WRITE_IP,
	CMD_HEART,
	CMD_UP_WARNS,
	CMD_UPDATE,
	CMD_ERROR
};
struct netmsg_DataHeader
{
	netmsg_DataHeader()
	{
		dataLength = sizeof(netmsg_DataHeader);
		cmd = CMD_ERROR;
	}
	short dataLength;
	short cmd;
};
struct netmsg_DtSet : public netmsg_DataHeader
{
	netmsg_DtSet()
	{
		dataLength = sizeof(netmsg_DtSet);
		cmd = CMD_DT_SET;
	}
	int result;
};
//DataPackage继承的方式在构造函数中初始化
struct netmsg_Heart : public netmsg_DataHeader
{
	netmsg_Heart()
	{
		dataLength = sizeof(netmsg_Heart);
		cmd = CMD_HEART;
	}
};
#endif