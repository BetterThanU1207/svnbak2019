#include "RawDataDeal.h"
#include "CELLMBus.h"

using namespace std;

RawDataDeal::RawDataDeal(const unsigned char* data, unsigned int length)
{
	_hexData.clear();
	//格式转换
	Ascii2Hex(data, length);
}
RawDataDeal::~RawDataDeal()
{
	_hexData.clear();
}
//直接存储不转换顺序
bool RawDataDeal::getData(string& ID, string& S2CID, string& S2CData)
{
	if (_hexData[1] == "00")//上位机
	{

	}
	else if (_hexData[1] == "01")//Mbus
	{
		CELLMBus mbus(_hexData);
		return mbus.getData(ID, S2CID, S2CData);
	}
	else if (_hexData[1] == "02")//lora
	{

	}
	//解析data
	//重整data	
	return false;
}

//转16进制
void RawDataDeal::Ascii2Hex(const unsigned char* data, unsigned int length)
{
	char buf[20];//长度尽量大一些，不然会Run-Time Check Failure #2 - Stack around the variable 'buf' was corrupte
	memset(buf, 0, sizeof(buf));

	for (int i = 0; i < length; i++)
	{
		//ASCII转hex
		sprintf(buf, "%02x", data[i]);
		_hexData.push_back(buf);
		memset(buf, 0, sizeof(buf));
	}
}
