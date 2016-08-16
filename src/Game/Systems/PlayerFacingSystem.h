#pragma once

#include <glm/glm.hpp>

#include "Framework/System.h"
#include "Renderer/UI/Label.h"

#include <btBulletDynamicsCommon.h>

#include <memory>

class PlayerFacingSystem : public System
{
public:
	PlayerFacingSystem(World& world, btDynamicsWorld* dynamicsWorld, const std::shared_ptr<Label>& label);
	void updateEntity(float dt, eid_t entity);
private:
	btDynamicsWorld* dynamicsWorld;
	std::shared_ptr<Label> label;
};