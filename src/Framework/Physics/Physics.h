#pragma once

#include <btBulletDynamicsCommon.h>

#include <unordered_map>

#include "Framework/World.h"
#include "CollisionResponder.h"

template < >
struct std::hash<std::pair<eid_t, eid_t>>
{
public:
	size_t operator()(std::pair<eid_t, eid_t> epair) const throw() {
		// I found this on the 'net, must be good right
		return std::hash<eid_t>()(epair.first) * 31 + std::hash<eid_t>()(epair.second);
	}
};

class Physics
{
public:
	Physics(btDynamicsWorld* dynamicsWorld);

	void fixedUpdate(btDynamicsWorld* world, float timeStep);

	void registerCollisionResponder(const std::shared_ptr<CollisionResponder>& collisionResponder);

	btPersistentManifold* getContact(eid_t e1, eid_t e2);
private:
	void handleContact(eid_t e1, eid_t e2, btPersistentManifold* contactManifold, CollisionResponseType type);

	struct PhysicsContact
	{
		PhysicsContact() : lastContactFrame(UINT64_MAX), contactManifold(nullptr) { }
		uint64_t lastContactFrame;
		btPersistentManifold* contactManifold;
	};

	std::unordered_map<std::pair<eid_t, eid_t>, PhysicsContact> contacts;
	std::vector<std::shared_ptr<CollisionResponder>> collisionResponders;
	btDynamicsWorld* dynamicsWorld;
	uint64_t currentFrame;

	const static unsigned framesBeforeUncaching;
};