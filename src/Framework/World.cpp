
#include "World.h"

World::eid_iterator::eid_iterator()
{ }

World::eid_iterator::eid_iterator(std::map<eid_t, ComponentBitmask>::iterator entityIterBegin,
	std::map<eid_t, ComponentBitmask>::iterator entityIterEnd,
	ComponentBitmask match)
{
	this->entityIter = entityIterBegin;
	this->entityIterEnd = entityIterEnd;
	this->match = match;

	while (this->entityIter != this->entityIterEnd && !this->entityIter->second.hasComponents(match)) {
		this->entityIter++;
	}
	this->entityIterBegin = this->entityIter;
}

eid_t World::eid_iterator::value()
{
	return entityIter->first;
}

void World::eid_iterator::next()
{
	do {
		entityIter++;
	} while (entityIter != entityIterEnd && !entityIter->second.hasComponents(match));
}

bool World::eid_iterator::atEnd()
{
	return this->entityIter == this->entityIterEnd;
}

void World::eid_iterator::reset()
{
	this->entityIter = this->entityIterBegin;
}

eid_t World::getNewEntity()
{
	eid_t id = nextEntityId++;
	entityComponents.emplace(id, ComponentBitmask());
	return id;
}

World::eid_iterator World::getEidIterator(ComponentBitmask match)
{
	return eid_iterator(entityComponents.begin(), entityComponents.end(), match);
}
