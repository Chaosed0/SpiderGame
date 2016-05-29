#pragma once

#include <unordered_set>
#include <typeinfo>
#include <vector>

#include "Entity.h"

class System
{
public:
	template <class T>
	void require();

	void update(float dt, std::vector<Entity>& entities);
	bool shouldUpdate(const Entity& entity) const;

	virtual void updateEntity(float dt, Entity& entity) = 0;
private:
	std::vector<size_t> requiredComponents;
};

template <class T>
void System::require()
{
	requiredComponents.push_back(typeid(T).hash_code());
}