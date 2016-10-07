#pragma once

#include "Framework/Component.h"
#include "Framework/ComponentBitmask.h"

#include <unordered_map>
#include <map>
#include <memory>
#include <cassert>
#include <string>

class Prefab;

typedef uint32_t eid_t;
typedef uint32_t cid_t;

class World
{
public:
	World() : nextComponentId(0), nextEntityId(0) { }
	
	/*!
	\brief Frees the memory from deleted entities. Can be called every frame.
	*/
	void cleanupEntities();

	/*!
	 \brief Gets the name of an entity.
	 */
	std::string getEntityName(eid_t eid) const;

	/*!
	 \brief Returns the first entity found with a given name.
	 */
	eid_t getEntityWithName(const std::string& name);

	/*!
	 \brief Constructs an entity from a prefab.
	 \param prefab The prefab to construct.
	 \param parent The entity to parent the newly constructed entity to.
	 \param userinfo User info to pass to component constructors.
	 \return The newly constructed entity.
	 */
	eid_t constructPrefab(const Prefab& prefab, eid_t parent = World::NullEntity, void* userinfo = nullptr);

	/*!
	 \brief Creates a new empty entity.
	 */
	eid_t getNewEntity(const std::string& name = "");

	/*!
	 \brief Deletes an entity from the world.
	*/
	void removeEntity(eid_t entity);

	/*!
	 \brief Checks if an entity has components.
	 To construct the ComponentBitmask, call getComponentId and set the corresponding bitmask bit.
	 */
	bool entityHasComponents(eid_t entity, const ComponentBitmask& bitmask) const;

	/*!
	 \brief Orders entities depending on the expected components.
	 This is used when you have two known entities, but don't know which order they are in (e.g. collision detection).
	 \param e1 Upon calling, one of the entities to check. On return, this is the entity that has components in b1.
	 \param e1 Upon calling, one of the entities to check. On return, this is the entity that has components in b2.
	 \param b1 The components of the first entity.
	 \param b1 The components of the second entity.
	 \return True if either e1 or e2 had the components in b1 and b2, false otherwise.
	 */
	bool orderEntities(eid_t& e1, eid_t& e2, const ComponentBitmask& b1, const ComponentBitmask& b2) const;

	/*!
	 \brief Adds a default-constructed component to an entity.
	 This is equivalent to calling getComponent with insert==true.
	 \param entity The entity to which the component is added.
	 \return The component which was added.
	 */
	template <class T>
	T* addComponent(eid_t entity);

	/*!
	 \brief Gets a component attached to an entity.
	 If insert==true and the there is no component of the passed type attached to the entity, the
	 method constructs one using the default constructor and attaches it.
	 \param entity The entity to which the component is added.
	 \param insert Whether or not to add a new component if it does not exist.
	 \return The component attached to the entity.
	 */
	template <class T>
	T* getComponent(eid_t entity, bool insert=false);

	/*!
	 \brief Returns the internal ID of a component.
	 */
	template <class T>
	cid_t getComponentId();

	/*!
	 \brief Returns all entities with the given component attached.
	 */
	template <class T>
	std::vector<eid_t> getEntitiesWithComponent();

	/*!
	 \brief Immediately deletes all entities.
	 */
	void clear();

	struct Entity {
		Entity(const std::string& name, ComponentBitmask components)
			: name(name), components(components), markedForDeletion(false) { }
		bool markedForDeletion;
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
	const static eid_t NullEntity;
private:
	typedef std::unordered_map<eid_t, std::unique_ptr<Component>> ComponentPool;

	template <class T>
	cid_t registerComponent();

	cid_t getComponentId(size_t typeidHash);
	cid_t registerComponent(size_t typeidHash);

	std::unordered_map<size_t, cid_t> componentIdMap;
	std::vector<ComponentPool> entityComponentMaps;
	std::map<eid_t, Entity> entities;

	cid_t nextComponentId;
	eid_t nextEntityId;

	ComponentBitmask getEntityBitmask(eid_t eid) const;
};

template <class T>
cid_t World::registerComponent()
{
	size_t hash = typeid(T).hash_code();
	return id;
}

template <class T>
cid_t World::getComponentId()
{
	size_t hash = typeid(T).hash_code();
	return getComponentId(hash);
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
	ComponentPool& componentPool = this->entityComponentMaps[cid];

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

template <class T>
std::vector<eid_t> World::getEntitiesWithComponent()
{
	cid_t cid = getComponentId<T>();
	ComponentPool& componentPool = this->entityComponentMaps[cid];
	std::vector<eid_t> entities;

	for (auto iter = componentPool.begin(); iter != componentPool.end(); ++iter) {
		entities.push_back(iter->first);
	}
	
	return entities;
}
