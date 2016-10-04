#pragma once

#include <vector>
#include <memory>

#include "Framework/ComponentConstructor.h"
#include "Framework/World.h"

class Prefab
{
public:
	void addConstructor(ComponentConstructor* constructor);
	std::vector<ComponentConstructorInfo> construct() const;
private:
	std::vector<std::shared_ptr<ComponentConstructor>> constructors;
};