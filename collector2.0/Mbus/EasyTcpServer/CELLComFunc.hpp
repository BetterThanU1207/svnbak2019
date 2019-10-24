#ifndef _CELL_COM_FUNC_HPP_
#define _CELL_COM_FUNC_HPP_

#include <string>
#include <vector>
#include <algorithm>
#include "CELL.hpp"

class CELLComFunc
{
//private:
//	CELLComFunc()
//	{
//		_taskServer.Start();
//	}
//	~CELLComFunc()
//	{
//		_taskServer.Close();
//	}
public:
	static CELLComFunc& Instance()
	{
		static CELLComFunc sComFunc;
		return sComFunc;
	}
	static void addCS16(std::string& str)
	{
		std::vector<std::string> strVec = CELLComFunc::Instance().str2vector(str);
		str += CELLComFunc::Instance().checkSum(strVec) + "16";		
	}

private:
	std::vector<std::string> str2vector(std::string str)
	{
		std::vector<std::string> vec;
		vec.clear();
		for (size_t i = 0; i < str.length(); )
		{
			vec.push_back(str.substr(i, 2));
			i += 2;
		}
		return vec;
	}

	std::string checkSum(std::vector<std::string> datas)
	{
		int ret = 0;

		for (size_t i = 0; i < datas.size(); i++)
		{
			ret += strtol(datas[i].c_str(), nullptr, 16);
		}
		ret = ret % 256;
		char buf[20];
		sprintf(buf, "%02x", ret);
		return buf;
	}

	std::string string_to_hex(std::vector<std::string> resistanceBytesAsString) //transfer string to hex-string
	{
		//std::vector<std::string> resistanceBytesAsString{ "00","01","10","11" };

		std::vector<unsigned char> bytes(resistanceBytesAsString.size());
		std::transform(
			resistanceBytesAsString.begin(),
			resistanceBytesAsString.end(),
			bytes.begin(),
			[](const std::string& str) {
				// Convert the string of length 2 to byte.
				// You could also use a stringstream with hex option.
				unsigned char byte = (str[0] - '0') << 1 | (str[1] - '0');
				return byte;
			});
		std::string str = "";
		for (size_t i = 0; i < resistanceBytesAsString.size(); i++)
		{
			str += bytes[i];
		}
		return str;
	}

private:
	//CellTaskServer _taskServer;
};



#endif // !_CELL_COM_FUNC_HPP_
