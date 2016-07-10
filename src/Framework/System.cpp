
#include "System.h"

System::System(World& world)
	: world(world)
{ }

void System::update(float dt)
{
	for (eid_iterator iterator = world.getEidIterator(requiredComponents);
		!iterator.atEnd();
		iterator.next())
	{
		updateEntity(dt, iterator.val());
	}
}
