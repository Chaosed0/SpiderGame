#pragma once

#include "Framework/System.h"
#include "Renderer/Renderer.h"

#include <btBulletDynamicsCommon.h>

class SpiderSystem : public System
{
public:
	SpiderSystem(World& world, btDynamicsWorld* dynamicsWorld, Renderer& renderer);
	virtual void updateEntity(float dt, eid_t entity);
private:
	void createHurtbox(const Transform& transform);

	Renderer& renderer;
	btDynamicsWorld* dynamicsWorld;

	static const float attackDistance;
};
