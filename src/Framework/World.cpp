
#include "World.h"

eid_iterator::eid_iterator(std::map<eid_t, ComponentBitmask>::iterator entityIter,
			std::map<eid_t, ComponentBitmask>::iterator entityIterEnd,
			ComponentBitmask match)
	: entityIterBegin(entityIter), entityIter(entityIter), entityIterEnd(entityIterEnd), match(match)
{
	while (this->entityIter != this->entityIterEnd && !this->entityIter->second.hasComponents(match)) {
		this->entityIter++;
	}
}

eid_t eid_iterator::val()
{
	return entityIter->first;
}

void eid_iterator::next()
{
	do {
		entityIter++;
	} while (entityIter != entityIterEnd && !entityIter->second.hasComponents(match));
}

bool eid_iterator::atEnd()
{
	return entityIter == entityIterEnd;
}

void eid_iterator::reset()
{
	entityIter = entityIterBegin;
	this->next();
}

eid_t World::getNewEntity()
{
	eid_t id = nextEntityId++;
	entityComponents.emplace(id, ComponentBitmask());
	return id;
}

eid_iterator World::getEidIterator(ComponentBitmask match)
{
	return eid_iterator(entityComponents.begin(), entityComponents.end(), match);
}
