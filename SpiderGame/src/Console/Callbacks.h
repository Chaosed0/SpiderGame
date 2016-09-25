#pragma once

#include <unordered_map>
#include <vector>
#include <functional>
#include <sstream>

typedef std::function<bool(const std::string& args)> Callback;

class CallbackMap
{
public:
	enum Error {
		CALLBACK_NOT_FOUND,
		CALLBACK_BAD_ARGS,
		CALLBACK_NO_ERROR
	};

	void setCallback(const std::string& callbackName, Callback callback);
	Error call(const std::string& callbackName, const std::string& args);

	static Callback defineCallback(std::function<void(void)> func);
	template <class T1>
	static Callback defineCallback(std::function<void(T1)> func);
	template <class T1, class T2>
	static Callback defineCallback(std::function<void(T1,T2)> func);
	template <class T1, class T2, class T3>
	static Callback defineCallback(std::function<void(T1,T2,T3)> func);
private:
	std::unordered_map<std::string, Callback> map;
};

template <class T1>
Callback CallbackMap::defineCallback(std::function<void(T1)> func)
{
	auto f = [func](const std::string& args) {
		T1 v1;
		std::stringstream sstream;
		sstream << args;
		sstream >> v1;
		if (sstream.fail()) {
			return false;
		}
		func(v1);
		return true;
	};
	return f;
}

template <class T1, class T2>
Callback CallbackMap::defineCallback(std::function<void(T1,T2)> func)
{
	auto f = [func](const std::string& args) {
		T1 v1;
		T2 v2;
		std::stringstream sstream;
		sstream << args;
		sstream >> v1 >> v2;
		if (sstream.fail()) {
			return false;
		}
		func(v1,v2);
		return true;
	};
	return f;
}

template <class T1, class T2, class T3>
Callback CallbackMap::defineCallback(std::function<void(T1,T2,T3)> func)
{
	auto f = [func](const std::string& args) {
		T1 v1;
		T2 v2;
		T3 v3;
		std::stringstream sstream;
		sstream << args;
		sstream >> v1 >> v2 >> v3;
		if (sstream.fail()) {
			return false;
		}
		func(v1,v2,v3);
		return true;
	};
	return f;
}
