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
	char buf[20];//���Ⱦ�����һЩ����Ȼ��Run-Time Check Failure #2 - Stack around the variable 'buf' was corrupte
	memset(buf, 0, sizeof(buf));
	for (int i = 0; i < length; i++)
	{
		//ASCIIתhex
		sprintf(buf, "%02x", data[i]);
		/*ʮ�������ַ�����תint
		char* str;
		long num = strtol(buf, &str, 16);*/
		_hexData.push_back(buf);
		memset(buf, 0, sizeof(buf));
	}
}
//�����ȡ����ԭʼ����
netmsg_DataHeader DataFromCollector::dataResult(const char* data, unsigned int length)
{
	//��ʽת��
	//string dataStr = data;//ת����ᵼ�����ݶ�ʧ
	//dataStr = dataStr.substr(0, length);
	Ascii2Hex(data, length);
	//��֤data
	Verify();
	//����data
	//����data
	//netmsg_LoginR* header =  new netmsg_LoginR;
	//header->result = 1;
	netmsg_LoginR header;
	header.result = 1;
	return header;
}

bool DataFromCollector::Verify()
{
	bool isCorrect = false;
	//�ۼӺ�У��
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
	//����У��
	return isCorrect;
}
