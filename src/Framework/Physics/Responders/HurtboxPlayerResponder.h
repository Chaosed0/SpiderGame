#pragma once

#include "Framework/Physics/CollisionResponder.h"

class HurtboxPlayerResponder : public CollisionResponder
{
public:
	HurtboxPlayerResponder(World& world);
	virtual void collisionBegan(eid_t e1, eid_t e2, btPersistentManifold* contactManifold);
	virtual void collisionEnded(eid_t e1, eid_t e2, btPersistentManifold* contactManifold);
private:
};