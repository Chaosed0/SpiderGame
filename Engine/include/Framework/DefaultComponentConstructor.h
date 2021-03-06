#pragma once

#include "Framework/ComponentConstructor.h"

template <class ComponentClass>
class DefaultComponentConstructor : public ComponentConstructor
{
public:
	DefaultComponentConstructor(const typename ComponentClass::Data& data);
	virtual ComponentConstructorInfo construct(World& world, eid_t parent, void* userinfo) const;
protected:
	typename ComponentClass::Data data;
};

template <class ComponentClass>
DefaultComponentConstructor<ComponentClass>::DefaultComponentConstructor(const typename ComponentClass::Data& data)
	: data(data)
{ }

template <class ComponentClass>
ComponentConstructorInfo DefaultComponentConstructor<ComponentClass>::construct(World& world, eid_t parent, void* userinfo) const
{
	ComponentClass* component = new ComponentClass();
	component->data = this->data;

	return ComponentConstructorInfo(component, typeid(ComponentClass).hash_code());
}