#pragma once

#include "Framework/System.h"

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

class FollowSystem : public System
{
public:
	FollowSystem(World& world, btDynamicsWorld* dynamicsWorld);
	void updateEntity(float dt, eid_t entity);
private:
	btDynamicsWorld* dynamicsWorld;
};