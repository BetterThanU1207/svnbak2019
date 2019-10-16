#include "DataFromCollector.h"

using namespace std;

DataFromCollector::DataFromCollector()
{
	_hexData.clear();
}

DataFromCollector::~DataFromCollector()
{
}

void DataFromCollector::Ascii2Hex(const unsigned char* data, unsigned int length)
{
	char buf[20];//长度尽量大一些，不然会Run-Time Check Failure #2 - Stack around the variable 'buf' was corrupte
	memset(buf, 0, sizeof(buf));

	for (int i = 0; i < length; i++)
	{
		//ASCII转hex
		sprintf(buf, "%02x", data[i]);
		/*十六进制字符数组转int
		char* str;
		long num = strtol(buf, &str, 16);*/
		/*处理ASCII不能显示的hex（会多出FFFFFF）
		string pushStr = buf;
		if (pushStr.length() == 8)
		{
			pushStr = pushStr.substr(6);
		}*/
		_hexData.push_back(buf);
		memset(buf, 0, sizeof(buf));
	}
}
//传入获取到的原始数据
netmsg_DataHeader DataFromCollector::dataResult(const unsigned char* data, unsigned int length)
{
	//格式转换
	//string dataStr = data;//转换后会导致数据丢失
	//dataStr = dataStr.substr(0, length);
	Ascii2Hex(data, length);
	//验证data
	if (Verify())
	{

	}
	//解析data
	//重整data
	//netmsg_LoginR* header =  new netmsg_LoginR;
	//header->result = 1;
	netmsg_LoginR header;
	header.result = 1;
	return header;
}

bool DataFromCollector::Verify()
{
	bool isCorrect = false;
	//累加和校验
	int rawChecksum = strtol(_hexData[_hexData.size() - 2].c_str(), nullptr, 16);
	int checksum = 0;
	for (int i = 0; i < _hexData.size() - 2; i++)
	{
		checksum += strtol(_hexData[i].c_str(), nullptr, 16);
	}
	if (rawChecksum != checksum % 256)
	{
		isCorrect = false;
		return isCorrect;
	}	
	//长度校验
#if 0
	//范新虎集抄器
	int dataLen = strtol(_hexData[3].c_str(), nullptr, 16) + strtol(_hexData[2].c_str(), nullptr, 16);
	if (dataLen == _hexData.size() - 12)
	{
		isCorrect = true;
	}
#else
	//汪衍赓集抄器——14：不计入数据长度的hex个数，10、11数据长度所在位置，低字节在前
	int dataLen = strtol(_hexData[11].c_str(), nullptr, 16) + strtol(_hexData[10].c_str(), nullptr, 16);
	if (dataLen == _hexData.size() - 14)
	{
		isCorrect = true;
	}
#endif
	return isCorrect;
}
