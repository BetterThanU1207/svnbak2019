#ifndef _RAW_DATA_DEAL_H_
#define _RAW_DATA_DEAL_H_

#include <vector>
#include <string>
//using namespace std;//会导致bind函数出错,迁移至cpp中

class RawDataDeal
{
public:
	RawDataDeal(const unsigned char* data, unsigned int length);
	~RawDataDeal();
	bool getData(std::string& ID, std::string& S2CID, std::string& S2CData);

private:
	void Ascii2Hex(const unsigned char* data, unsigned int length);

private:
	//原始十六进制数据
	std::vector<std::string> _hexData;

};
#endif // !_RAW_DATA_DEAL_H_

