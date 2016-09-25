#pragma once

#include <unordered_map>
#include <memory>
#include <functional>

#include "Optional.h"

template <class T>
class HandlePool
{
public:
	HandlePool() : nextHandle(0), deleter(pool) { }
	using Pool = std::unordered_map<uint32_t, T>;
	using Handle = std::shared_ptr<uint32_t>;

	typename Handle getNewHandle(const T& obj);
	typename Pool::iterator begin();
	typename Pool::iterator end();
	std::experimental::optional<std::reference_wrapper<T>> get(Handle handle);

	const static Handle invalidHandle;
private:
	class HandleDeleter {
	public:
		HandleDeleter(Pool& pool) : pool(pool) { }
		void operator() (uint32_t* handle) const;
		Pool& pool;
	};
	HandleDeleter deleter;
	uint32_t nextHandle;
	Pool pool;
};

template<class T>
const typename HandlePool<T>::Handle HandlePool<T>::invalidHandle(std::make_shared<uint32_t>(UINT32_MAX));

template <class T>
typename HandlePool<T>::Handle HandlePool<T>::getNewHandle(const T& obj)
{
	this->pool.emplace(this->nextHandle, obj);
	HandlePool<T>::Handle handle(new uint32_t(this->nextHandle), deleter);
	this->nextHandle++;
	return handle;
}

template <class T>
void HandlePool<T>::HandleDeleter::operator() (uint32_t* handle) const
{
	this->pool.erase(*handle);
	delete handle;
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
std::experimental::optional<std::reference_wrapper<T>> HandlePool<T>::get(Handle handle)
{
	if (handle == nullptr || handle == invalidHandle) {
		return std::experimental::optional<std::reference_wrapper<T>>();
	}

	auto iter = this->pool.find(*handle);
	if (iter == this->pool.end()) {
		return std::experimental::optional<std::reference_wrapper<T>>();
	}

	return std::experimental::optional<std::reference_wrapper<T>>(iter->second);
}
