#pragma once

#include "Framework/Component.h"

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
	virtual ComponentConstructorInfo construct() const = 0;
private:
};