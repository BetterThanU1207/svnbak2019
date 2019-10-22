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
	char buf[20];//���Ⱦ�����һЩ����Ȼ��Run-Time Check Failure #2 - Stack around the variable 'buf' was corrupte
	memset(buf, 0, sizeof(buf));

	for (int i = 0; i < length; i++)
	{
		//ASCIIתhex
		sprintf(buf, "%02x", data[i]);
		_hexData.push_back(buf);
		memset(buf, 0, sizeof(buf));
	}
}
//�����ȡ����ԭʼ����
netmsg_DataHeader* DataFromCollector::dataResult(const unsigned char* data, unsigned int length)
{
	//��ʽת��
	Ascii2Hex(data, length);
	//��֤data
	if (Verify())
	{

	}
	//����data
	//����data
	netmsg_DtSet* header =  new netmsg_DtSet;
	header->result = 1;
	header->realID = (char*)(_hexData[9] + _hexData[8] + _hexData[7] + _hexData[6] + _hexData[5]).data();
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
#if 0
	//���»�������
	int dataLen = strtol(_hexData[3].c_str(), nullptr, 16) + strtol(_hexData[2].c_str(), nullptr, 16);
	if (dataLen == _hexData.size() - 12)
	{
		isCorrect = true;
	}
#else
	//�����ټ���������14�����������ݳ��ȵ�hex������10��11���ݳ�������λ�ã����ֽ���ǰ
	int dataLen = strtol(_hexData[11].c_str(), nullptr, 16) + strtol(_hexData[10].c_str(), nullptr, 16);
	if (dataLen == _hexData.size() - 14)
	{
		isCorrect = true;
	}
#endif
	return isCorrect;
}
