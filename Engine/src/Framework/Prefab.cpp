
#include "Framework/Prefab.h"

Prefab::Prefab()
{ }

Prefab::Prefab(const std::string& name)
	: name(name)
{ }

void Prefab::addConstructor(ComponentConstructor* constructor)
{
	constructors.push_back(std::shared_ptr<ComponentConstructor>(constructor));
}

std::vector<ComponentConstructorInfo> Prefab::construct(World& world, eid_t parent, void* userinfo) const
{
	std::vector<ComponentConstructorInfo> infos;
	for (unsigned i = 0; i < constructors.size(); i++) {
		infos.emplace_back(constructors[i]->construct(world, parent, userinfo));
	}
	return infos;
}

void Prefab::finish(World& world, eid_t entity) const
{
	for (unsigned i = 0; i < constructors.size(); i++) {
		constructors[i]->finish(world, entity);
	}
}

void Prefab::addChild(const std::shared_ptr<Prefab>& prefab)
{
	this->childPrefabs.push_back(prefab);
}

std::vector<std::shared_ptr<Prefab>> Prefab::getChildPrefabs() const
{
	return childPrefabs;
}

void Prefab::setName(const std::string& name)
{
	this->name = name;
}

std::string Prefab::getName() const
{
	return this->name;
}