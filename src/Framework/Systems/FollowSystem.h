#pragma once

#include "Framework/System.h"

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

class FollowSystem : public System
{
public:
	FollowSystem(btDynamicsWorld* world);
	void updateEntity(float dt, Entity& entity);
private:
	btDynamicsWorld* world;
};