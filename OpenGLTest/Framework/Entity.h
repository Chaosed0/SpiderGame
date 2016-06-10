#pragma once

#include <unordered_map>
#include <typeinfo>
#include <memory>

#include "Component.h"

class Entity
{
public:
	template <class T>
	T* addComponent();

	template <class T>
	T* getComponent();

	bool hasComponent(size_t typeinfo) const;
private:
	std::unordered_map<size_t, std::shared_ptr<Component>> componentTypeMap;
};

template <class T>
T* Entity::addComponent()
{
	size_t hash = typeid(T).hash_code();
	componentTypeMap.emplace(hash, std::shared_ptr<Component>(static_cast<Component*>(new T)));
	return static_cast<T*>(componentTypeMap[hash].get());
	return NULL;
}

template <class T>
T* Entity::getComponent()
{
	auto iter = componentTypeMap.find(typeid(T).hash_code());
	return iter == componentTypeMap.end() ? nullptr : static_cast<T*>(iter->second.get());
	return NULL;
}
