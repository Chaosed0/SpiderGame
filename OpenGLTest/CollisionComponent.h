#pragma once

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <memory>

struct CollisionComponent : public Component
{
	std::shared_ptr<btRigidBody> body;
};