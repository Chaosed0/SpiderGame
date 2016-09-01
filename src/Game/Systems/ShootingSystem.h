#pragma once

#include "Framework/System.h"
#include "Framework/EventManager.h"
#include "Renderer/Shader.h"
#include "Renderer/Renderer.h"

class btDynamicsWorld;
class Shader;

class ShootingSystem : public System
{
public:
	ShootingSystem(World& world, btDynamicsWorld* dynamicsWorld, Renderer& renderer, EventManager& eventManager);
	virtual void updateEntity(float dt, eid_t entity);
private:
	btDynamicsWorld* dynamicsWorld;
	Renderer& renderer;
	EventManager& eventManager;
	Shader lineShader;

	Renderer::ModelHandle bulletMeshHandle;
};