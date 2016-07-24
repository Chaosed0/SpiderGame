#pragma once

#include "Component.h"
#include "ComponentBitmask.h"

#include <unordered_map>
#include <map>
#include <memory>
#include <cassert>
#include <string>

typedef uint32_t eid_t;
typedef uint32_t cid_t;

class World
{
public:
	World() : nextComponentId(0), nextEntityId(0) { }

	template <class T>
	void registerComponent();
	
	eid_t getNewEntity(const std::string& name = "");
	std::string getEntityName(eid_t t);

	template <class T>
	T* addComponent(eid_t entity);

	template <class T>
	T* getComponent(eid_t entity, bool insert=false);

	template <class T>
	cid_t getComponentId();

	struct Entity {
		Entity(const std::string& name, ComponentBitmask components)
			: name(name), components(components) { }
		std::string name;
		ComponentBitmask components;
	};

	class eid_iterator
	{
	public:
		eid_iterator();
		eid_iterator(std::map<eid_t, Entity>::iterator entityIterBegin,
			std::map<eid_t, Entity>::iterator entityIterEnd,
			ComponentBitmask match);
		eid_t value();
		void reset();
		void next();
		bool atEnd();
	private:
		std::map<eid_t, Entity>::iterator entityIterBegin;
		std::map<eid_t, Entity>::iterator entityIter;
		std::map<eid_t, Entity>::iterator entityIterEnd;
		ComponentBitmask match;
	};

	eid_iterator getEidIterator(ComponentBitmask match);
private:
	std::unordered_map<size_t, cid_t> componentIdMap;
	std::vector<std::unordered_map<eid_t, std::shared_ptr<Component>>> entityComponentMaps;
	std::map<eid_t, Entity> entities;

	cid_t nextComponentId;
	eid_t nextEntityId;
};

template <class T>
void World::registerComponent()
{
	cid_t id = nextComponentId++;
	componentIdMap.emplace(typeid(T).hash_code(), id);
	entityComponentMaps.push_back(std::unordered_map<cid_t, std::shared_ptr<Component>>());
	assert(componentIdMap.size() == entityComponentMaps.size());
}

template <class T>
cid_t World::getComponentId()
{
	size_t hash = typeid(T).hash_code();
	auto iter = componentIdMap.find(hash);
	if (iter == componentIdMap.end()) {
		throw "Tried to add unregistered component";
	}
	return iter->second;
}
	
template <class T>
T* World::addComponent(eid_t entity)
{
	return this->getComponent<T>(entity, true);
}

template <class T>
T* World::getComponent(eid_t entity, bool insert)
{
	cid_t cid = getComponentId<T>();
	std::unordered_map<eid_t, std::shared_ptr<Component>>& componentPool = this->entityComponentMaps[cid];

	auto iter = componentPool.find(entity);
	if (iter == componentPool.end()) {
		if (!insert) {
			return nullptr;
		} else {
			std::unique_ptr<Component> newComponent(static_cast<Component*>(new T()));
			auto emplaceIter = componentPool.emplace(entity, std::move(newComponent));
			iter = emplaceIter.first;

			auto entityIter = entities.find(entity);
			if (entityIter == entities.end()) {
				throw "Tried to add component to nonexistent entity";
			}

			entityIter->second.components.setBit(cid, true);
		}
	}

	return static_cast<T*>(iter->second.get());
}
