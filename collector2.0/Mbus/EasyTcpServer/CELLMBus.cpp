#include "CELLMBus.h"
#include "CELLComFunc.hpp"
using namespace std;
CELLMBus::CELLMBus(std::vector<std::string> data)
{
	_data.clear();
	_data = data;
}
CELLMBus::~CELLMBus()
{
	_data.clear();
}
bool CELLMBus::getData(std::string& ID, std::string& S2CID, std::string& S2CData)
{
	if (1)
	{
		ID = _data[9] + _data[8] + _data[7] + _data[6] + _data[5];		
		//处理数据
		OnDeal(S2CID, S2CData);
		return true;
	}
	return false;
}
//验证校验位和数据长度
bool CELLMBus::Verify()
{
	bool isCorrect = false;
	//累加和校验
	int rawChecksum = strtol(_data[_data.size() - 2].c_str(), nullptr, 16);
	int checksum = 0;
	//校验和不包括起始符
	for (int i = 1; i < _data.size() - 2; i++)
	{
		checksum += strtol(_data[i].c_str(), nullptr, 16);
	}
	if (rawChecksum != checksum % 256)
	{
		isCorrect = false;
		return isCorrect;
	}
	//长度校验
#if 1
	//范新虎集抄器
	int dataLen = strtol(_data[3].c_str(), nullptr, 16) + strtol(_data[2].c_str(), nullptr, 16);
	if (dataLen == _data.size() - 12)
	{
		isCorrect = true;
	}
#else
	//汪衍赓集抄器——14：不计入数据长度的hex个数，10、11数据长度所在位置，低字节在前
	int dataLen = strtol(_data[11].c_str(), nullptr, 16) + strtol(_data[10].c_str(), nullptr, 16);
	if (dataLen == _data.size() - 14)
	{
		isCorrect = true;
	}
#endif
	return isCorrect;
}
//处理网络消息
void CELLMBus::OnDeal(std::string& S2CID, std::string& S2CData)
{	
	int func = strtol(_data[4].c_str(), nullptr, 16);
	switch (func)
	{
	case setCollectTime:
		break;
	case donwloadWmID:
		//总帧数
		//第几帧
		//本帧数量
		break;
	case wmDataUpload:
		//总帧数
		//第几帧
		//本帧数量
		//每块表表号、流量、抄表时间
		break;
	case readWmData:
		//表地址
		//流量
		break;
	case readCollectorParas:
		break;
	case readCollectorWmIDs:
		break;
	case setCollectorIP:
		break;
	case heartBeat:
		S2CID = _data[9] + _data[8] + _data[7] + _data[6] + _data[5];
		S2CData = "01060008" + _data[5] + _data[6] + _data[7] + _data[8] + _data[9] + "130A1603161E";
		CELLComFunc::addCS16(S2CData);
		S2CData = "68" + S2CData;
		break;
	case warnInfoUpload:
		break;
	case updateCollector:
		break;
	default:
		break;
	}	
}