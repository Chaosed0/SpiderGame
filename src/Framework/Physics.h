#pragma once

#include <btBulletDynamicsCommon.h>

#include <unordered_map>

#include "World.h"

class Physics
{
public:
	Physics(btDynamicsWorld* dynamicsWorld);

	void fixedUpdate(btDynamicsWorld* world, float timeStep);

	btPersistentManifold* getContact(eid_t e1, eid_t e2);
private:
	struct PhysicsContact
	{
		PhysicsContact() : lastContactFrame(0), contactManifold(nullptr) { }
		uint64_t lastContactFrame;
		btPersistentManifold* contactManifold;
	};

	std::unordered_map<eid_t, std::unordered_map<eid_t, PhysicsContact>> contacts;
	btDynamicsWorld* dynamicsWorld;
	uint64_t currentFrame;
};