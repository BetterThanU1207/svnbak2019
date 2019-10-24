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
		//�Թ�������Ϊ��λ��ID
		ID = _data[4];
		//��������
		//Ҫ�����ļ�����
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
//��֤У��λ�����ݳ���
bool CELLWeb::Verify()
{
	bool isCorrect = false;
	//�ۼӺ�У��
	int rawChecksum = strtol(_data[_data.size() - 2].c_str(), nullptr, 16);
	int checksum = 0;
	//У��Ͳ�������ʼ��
	for (int i = 1; i < _data.size() - 2; i++)
	{
		checksum += strtol(_data[i].c_str(), nullptr, 16);
	}
	if (rawChecksum != checksum % 256)
	{
		isCorrect = false;
		return isCorrect;
	}
	//����У��
#if 1
	//���»�������
	int dataLen = strtol(_data[3].c_str(), nullptr, 16) + strtol(_data[2].c_str(), nullptr, 16);
	if (dataLen == _data.size() - 12)
	{
		isCorrect = true;
	}
#else
	//�����ټ���������14�����������ݳ��ȵ�hex������10��11���ݳ�������λ�ã����ֽ���ǰ
	int dataLen = strtol(_data[11].c_str(), nullptr, 16) + strtol(_data[10].c_str(), nullptr, 16);
	if (dataLen == _data.size() - 14)
	{
		isCorrect = true;
	}
#endif
	return isCorrect;
}