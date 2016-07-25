
#include "World.h"

World::eid_iterator::eid_iterator()
{ }

World::eid_iterator::eid_iterator(std::map<eid_t, Entity>::iterator entityIterBegin,
	std::map<eid_t, Entity>::iterator entityIterEnd,
	ComponentBitmask match)
{
	this->entityIter = entityIterBegin;
	this->entityIterEnd = entityIterEnd;
	this->match = match;

	while (this->entityIter != this->entityIterEnd && !this->entityIter->second.components.hasComponents(match)) {
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
	} while (entityIter != entityIterEnd && !entityIter->second.components.hasComponents(match));
}

bool World::eid_iterator::atEnd()
{
	return this->entityIter == this->entityIterEnd;
}

void World::eid_iterator::reset()
{
	this->entityIter = this->entityIterBegin;
}

eid_t World::getNewEntity(const std::string& name)
{
	eid_t id = nextEntityId++;
	std::string actualName = name;
	if (name.length() == 0) {
		actualName = "Entity " + id;
	}

	entities.emplace(id, World::Entity(actualName, ComponentBitmask()));
	return id;
}

void World::removeEntity(eid_t entity)
{
	auto entityIter = entities.find(entity);
	if (entityIter == entities.end()) {
		return;
	}

	entityIter->second.markedForDeletion = true;
}

void World::cleanupEntities()
{
	auto iter = entities.begin();
	while (iter != entities.end())
	{
		if (iter->second.markedForDeletion) {
			eid_t eid = iter->first;
			for (unsigned i = 0; i < entityComponentMaps.size(); i++) {
				ComponentPool& componentPool = entityComponentMaps[i];
				auto componentIter = componentPool.find(eid);
				if (componentIter != componentPool.end()) {
					componentPool.erase(componentIter);
				}
			}

			iter = entities.erase(iter);
		} else {
			++iter;
		}
	}
}

std::string World::getEntityName(eid_t eid)
{
	auto iter = entities.find(eid);
	if (iter != entities.end()) {
		return iter->second.name;
	}
	return "";
}

World::eid_iterator World::getEidIterator(ComponentBitmask match)
{
	return eid_iterator(entities.begin(), entities.end(), match);
}
