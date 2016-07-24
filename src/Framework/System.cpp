
#include "System.h"

System::System(World& world)
	: world(world)
{ }

void System::update(float dt)
{
	entityIterator = world.getEidIterator(requiredComponents);
	while(!entityIterator.atEnd())
	{
		updateEntity(dt, entityIterator.value());
		entityIterator.next();
	}
}
