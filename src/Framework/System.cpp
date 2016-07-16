
#include "System.h"

System::System(World& world)
	: world(world)
{ }

void System::requireFinished()
{
	entityIterator = world.getEidIterator(requiredComponents);
}

void System::update(float dt)
{
	entityIterator.reset();
	while(!entityIterator.atEnd())
	{
		updateEntity(dt, entityIterator.value());
		entityIterator.next();
	}
}
