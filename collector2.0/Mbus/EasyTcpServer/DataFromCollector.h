#ifndef _DATA_FROM_COLLECTOR_H_
#define _DATA_FROM_COLLECTOR_H_

#include <vector>
#include"CELL.hpp"
//using namespace std;//�ᵼ��bind��������,Ǩ����cpp��

class DataFromCollector
{
public:
	DataFromCollector();
	~DataFromCollector();
	//�����ȡ����ԭʼ����
	netmsg_DataHeader dataResult(const char* data, unsigned int length);

private:
	void Ascii2Hex(const char* data, unsigned int length);
	bool Verify();

private:
	//ԭʼʮ����������
	std::vector<std::string> _hexData;

};
#endif // !_DATA_FROM_COLLECTOR_H_

