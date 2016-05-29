#pragma once

#include <unordered_map>
#include <typeinfo>
#include <memory>

#include "Component.h"

class Entity
{
public:
	template <class T>
	std::shared_ptr<T> addComponent();

	template <class T>
	std::shared_ptr<T> getComponent();

	bool hasComponent(size_t typeinfo) const;
private:
	std::unordered_map<size_t, std::shared_ptr<Component>> componentTypeMap;
};

template <class T>
std::shared_ptr<T> Entity::addComponent()
{
	std::shared_ptr<T> newComponent(new T());
	componentTypeMap[typeid(T).hash_code()] = newComponent;
	return newComponent;
}

template <class T>
std::shared_ptr<T> Entity::getComponent()
{
	auto iter = componentTypeMap.find(typeid(T).hash_code());
	return iter == componentTypeMap.end() ? std::shared_ptr<T>(nullptr) : std::static_pointer_cast<T>(iter->second);
}
