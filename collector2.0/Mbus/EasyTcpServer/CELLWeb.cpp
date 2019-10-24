#include "CELLWeb.h"

CELLWeb::CELLWeb(std::vector<std::string> data)
{

}
CELLWeb::~CELLWeb()
{

}
bool CELLWeb::getData(std::string& ID, std::string& S2CID, std::string& S2CData)
{
	if (1)
	{
		//以功能码作为上位机ID
		ID = _data[4];
		//处理数据
		//要操作的集抄器
		S2CID = _data[9] + _data[8] + _data[7] + _data[6] + _data[5];
		S2CData = "";
		for (size_t i = 0; i < _data.size(); i++)
		{
			S2CData += _data[i];
		}		
		return true;
	}
	return false;
}
//验证校验位和数据长度
bool CELLWeb::Verify()
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