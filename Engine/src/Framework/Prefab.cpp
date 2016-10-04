
#include "Framework/Prefab.h"

void Prefab::addConstructor(ComponentConstructor* constructor)
{
	constructors.push_back(std::shared_ptr<ComponentConstructor>(constructor));
}

std::vector<ComponentConstructorInfo> Prefab::construct() const
{
	std::vector<ComponentConstructorInfo> infos;
	for (unsigned i = 0; i < constructors.size(); i++) {
		infos.emplace_back(constructors[i]->construct());
	}
	return infos;
}