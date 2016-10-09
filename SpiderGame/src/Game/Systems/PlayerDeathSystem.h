#pragma once

#include "Framework/System.h"
#include "Framework/EventManager.h"

class PlayerDeathSystem : public System
{
public:
	PlayerDeathSystem(World& world, EventManager& eventManager);
	virtual void updateEntity(float dt, eid_t entity);
private:
	EventManager& eventManager;
};
