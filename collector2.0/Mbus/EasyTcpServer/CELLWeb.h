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

class CELLWeb
{
public:
	CELLWeb(std::vector<std::string> data);
	~CELLWeb();
	bool getData(std::string& ID, std::string& S2CID, std::string& S2CData);
private:	
	bool Verify();
private:
	std::vector<std::string> _data;
};

