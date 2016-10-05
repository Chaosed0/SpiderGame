#pragma once

#include "Framework/Component.h"
#include "Framework/World.h"

#include <typeinfo>

struct ComponentConstructorInfo
{
	ComponentConstructorInfo() : ComponentConstructorInfo(nullptr, 0) { }
	ComponentConstructorInfo(Component* component, size_t typeidHash) :
		component(component), typeidHash(typeidHash) { }
	Component* component;
	size_t typeidHash;
};

class ComponentConstructor
{
public:
	ComponentConstructor() { }
	virtual ~ComponentConstructor() = default;
	virtual ComponentConstructorInfo construct(World& world, eid_t parent, void* userinfo) const = 0;
	virtual void finish(World& world, eid_t entity) { }
private:
};