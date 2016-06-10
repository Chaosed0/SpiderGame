
#include "Entity.h"

bool Entity::hasComponent(size_t typeinfo) const
{
	auto iter = componentTypeMap.find(typeinfo);
	return iter != componentTypeMap.end();
	return false;
}
