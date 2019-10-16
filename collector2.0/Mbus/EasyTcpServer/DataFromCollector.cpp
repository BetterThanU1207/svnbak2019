#include "DataFromCollector.h"

using namespace std;

DataFromCollector::DataFromCollector()
{
	_hexData.clear();
}

DataFromCollector::~DataFromCollector()
{
}

int hexcharToInt(char c)
{
	if (c >= '0' && c <= '9') return (c - '0');
	if (c >= 'A' && c <= 'F') return (c - 'A' + 10);
	if (c >= 'a' && c <= 'f') return (c - 'a' + 10);
	return 0;
}

void DataFromCollector::Ascii2Hex(const char* data, unsigned int length)
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
		_hexData.push_back(buf);
		memset(buf, 0, sizeof(buf));
	}
}
//传入获取到的原始数据
netmsg_DataHeader DataFromCollector::dataResult(const char* data, unsigned int length)
{
	//格式转换
	//string dataStr = data;//转换后会导致数据丢失
	//dataStr = dataStr.substr(0, length);
	Ascii2Hex(data, length);
	//验证data
	Verify();
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
	return isCorrect;
}
