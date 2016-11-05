
#include "Config.h"

#include <fstream>

Config::Config()
{
}

ConfigParsingError Config::loadConfig(const std::string& configPath)
{
	std::ifstream instream;
	instream.open(configPath);

	if (!instream.is_open()) {
		return ConfigParsingError_FileNotFound;
	}

	while (!instream.eof()) {
		std::string line;
		instream >> line;

		int eq = line.find_first_of('=');

		if (eq < 0) {
			return ConfigParsingError_SyntaxError;
		}

		std::string key = line.substr(0, eq);
		std::string value = line.substr(eq+1);

		valMap[key] = value;
	}

	return ConfigParsingError_None;
}

template <>
std::string Config::getValue<std::string>(const std::string& varName, const std::string& defaultValue)
{
	auto iter = valMap.find(varName);
	if (iter == valMap.end()) {
		return defaultValue;
	}
	return iter->second;
}
