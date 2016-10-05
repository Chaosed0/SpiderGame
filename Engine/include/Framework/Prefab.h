#pragma once

#include <vector>
#include <memory>

#include "Framework/ComponentConstructor.h"
#include "Framework/World.h"

class Prefab
{
public:
	Prefab();
	Prefab(const std::string& name);

	void addConstructor(ComponentConstructor* constructor);
	std::vector<ComponentConstructorInfo> construct(World& world, eid_t parent, void* userinfo) const;
	void finish(World& world, eid_t entity) const;

	void addChild(const std::shared_ptr<Prefab>& prefab);
	std::vector<std::shared_ptr<Prefab>> getChildPrefabs() const;

	void setName(const std::string& name);
	std::string getName() const;
private:
	std::vector<std::shared_ptr<ComponentConstructor>> constructors;
	std::vector<std::shared_ptr<Prefab>> childPrefabs;
	std::string name;
};