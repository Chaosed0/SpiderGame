#pragma once

#include <glm/glm.hpp>

#include "Framework/System.h"
#include "Renderer/UI/Label.h"
#include "Renderer/UI/UIRenderer.h"

#include <btBulletDynamicsCommon.h>

#include <memory>

class PlayerFacingSystem : public System
{
public:
	PlayerFacingSystem(World& world, btDynamicsWorld* dynamicsWorld);
	void updateEntity(float dt, eid_t entity);
private:
	btDynamicsWorld* dynamicsWorld;
	std::shared_ptr<Label> label;
};