#pragma once

#include <unordered_set>
#include <typeinfo>
#include <vector>
#include <cassert>

#include "World.h"

/*! Abstract base class for the System in ECS. Matches entities by component
	and operates upon those components. */
class System
{
public:
	System(World& world);

	/*!
	 * \brief Updates all corresponding entities in the world (passed in the constructor).
	 * \param dt The time which passed since the last call to this function.
	 */
	void update(float dt);

	/*!
	 * \brief Updates a single entity. Must be overriden in subclasses.
	 * \param dt The time which passed since the last call to update.
	 * \param entity The entity which should be updated.
	 */
	virtual void updateEntity(float dt, eid_t entity) = 0;
protected:
	/*!
	 * \brief Called by subclasses in their constructors. updateEntity will only
		be called with entities which have the components passed in require.
	 */
	template<class T>
	void require();

	/*! The world which this system operates on. */
	World& world;
private:
	/*! The bitmask of component IDs which is generated from calls to require. */
	ComponentBitmask requiredComponents;

	/*! Iterator we use to get entities to update. */
	World::eid_iterator entityIterator;
};

template <class T>
void System::require()
{
	cid_t cid = world.getComponentId<T>();
	requiredComponents.setBit(cid, true);
}