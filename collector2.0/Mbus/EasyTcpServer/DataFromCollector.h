#ifndef _DATA_FROM_COLLECTOR_H_
#define _DATA_FROM_COLLECTOR_H_

#include <vector>
#include"CELL.hpp"
//using namespace std;//会导致bind函数出错,迁移至cpp中

class DataFromCollector
{
public:
	DataFromCollector();
	~DataFromCollector();
	//传入获取到的原始数据
	netmsg_DataHeader dataResult(const char* data, unsigned int length);

private:
	void Ascii2Hex(const char* data, unsigned int length);
	bool Verify();

private:
	//原始十六进制数据
	std::vector<std::string> _hexData;

};
#endif // !_DATA_FROM_COLLECTOR_H_

