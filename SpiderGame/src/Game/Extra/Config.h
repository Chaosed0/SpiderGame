#pragma once

#include <string>
#include <unordered_map>
#include <sstream>

enum ConfigParsingError
{
	ConfigParsingError_None,
	ConfigParsingError_FileNotFound,
	ConfigParsingError_SyntaxError
};

class Config
{
public:
	Config();

	ConfigParsingError loadConfig(const std::string& configPath);

	template <typename T>
	T getValue(const std::string& varName, const T& defaultValue);
private:
	std::unordered_map<std::string, std::string> valMap;
};

template <typename T>
T Config::getValue(const std::string& varName, const T& defaultValue)
{
	auto iter = valMap.find(varName);
	if (iter == valMap.end()) {
		return defaultValue;
	}

	T value;
	std::stringstream sstream;
	sstream << iter->second;
	sstream >> value;
	return value;
}
