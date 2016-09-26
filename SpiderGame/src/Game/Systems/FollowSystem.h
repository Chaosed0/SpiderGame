#pragma once

#include "Framework/System.h"
#include "Environment/Room.h"

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

class FollowSystem : public System
{
public:
	FollowSystem(World& world, btDynamicsWorld* dynamicsWorld, Room room);
	void updateEntity(float dt, eid_t entity);
private:
	btDynamicsWorld* dynamicsWorld;
	Room room;

	bool findPath(const glm::vec3& start, const glm::vec3& finalTarget, std::vector<glm::vec3>& path);
};