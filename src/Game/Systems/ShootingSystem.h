#pragma once

#include "Framework/System.h"
#include "Renderer/Shader.h"

class btDynamicsWorld;
class Shader;
class Renderer;

class ShootingSystem : public System
{
public:
	ShootingSystem(World& world, btDynamicsWorld* dynamicsWorld, Renderer& renderer);
	virtual void updateEntity(float dt, eid_t entity);
private:
	btDynamicsWorld* dynamicsWorld;
	Renderer& renderer;
	Shader lineShader;
};