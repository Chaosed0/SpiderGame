
#include "Callbacks.h"

void CallbackMap::setCallback(const std::string& callbackName, Callback callback)
{
	map[callbackName] = callback;
}

CallbackMap::Error CallbackMap::call(const std::string& callbackName, const std::string& args)
{
	auto iter = map.find(callbackName);
	if (iter == map.end()) {
		return CALLBACK_NOT_FOUND;
	}

	bool success = iter->second(args);
	if (!success) {
		return CALLBACK_BAD_ARGS;
	}

	return CALLBACK_NO_ERROR;
}

Callback CallbackMap::defineCallback(std::function<void(void)> func)
{
	auto f = [func](const std::string& args) {
		func();
		return true;
	};
	return f;
}
