#pragma once
#include <vector>
#include <string>

enum FuncCode {
	setCollectTime = 1,
	donwloadWmID,
	wmDataUpload,
	readWmData,
	readCollectorParas,
	readCollectorWmIDs,
	setCollectorIP,
	heartBeat,
	warnInfoUpload,
	updateCollector,
	readAllData = 88,
	readWarnData = 99
};
//mbus集抄器数据处理
class CELLMBus
{
public:
	CELLMBus(std::vector<std::string> data);
	~CELLMBus();
	bool getData(std::string& ID, std::string& S2CID, std::string& S2CData);
private:
	void OnDeal(std::string& S2CID, std::string& S2CData);
	bool Verify();
private:
	std::vector<std::string> _data;
};

