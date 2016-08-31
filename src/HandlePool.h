#pragma once

#include <unordered_map>

#include "Optional.h"

template <class T>
class HandlePool
{
public:
	HandlePool() : nextHandle(0) { }
	using Pool = std::unordered_map<uint32_t, T>;

	uint32_t getNewHandle(const T& obj);
	void freeHandle(uint32_t handle);
	typename Pool::iterator begin();
	typename Pool::iterator end();
	std::experimental::optional<std::reference_wrapper<T>> get(uint32_t handle);
private:
	uint32_t nextHandle;
	Pool pool;
};

template <class T>
uint32_t HandlePool<T>::getNewHandle(const T& obj)
{
	this->pool.emplace(this->nextHandle, obj);
	return this->nextHandle++;
}

template <class T>
void HandlePool<T>::freeHandle(uint32_t handle)
{
	this->pool.erase(handle);
}

template<class T>
typename HandlePool<T>::Pool::iterator HandlePool<T>::begin()
{
	return this->pool.begin();
}

template <class T>
typename HandlePool<T>::Pool::iterator HandlePool<T>::end()
{
	return this->pool.end();
}

template <class T>
std::experimental::optional<std::reference_wrapper<T>> HandlePool<T>::get(uint32_t handle)
{
	auto iter = this->pool.find(handle);
	if (iter == this->pool.end()) {
		return std::experimental::optional<std::reference_wrapper<T>>();
	}
	return std::experimental::optional<std::reference_wrapper<T>>(iter->second);
}
