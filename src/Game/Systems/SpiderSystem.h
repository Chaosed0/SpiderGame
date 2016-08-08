#pragma once

#include "Framework/System.h"
#include "Renderer/Shader.h"

#include <glm/glm.hpp>

class Renderer;
class Transform;
class btDynamicsWorld;

class SpiderSystem : public System
{
public:
	SpiderSystem(World& world, btDynamicsWorld* dynamicsWorld, Renderer& renderer);
	virtual void updateEntity(float dt, eid_t entity);

	Shader debugShader;
private:
	void createHurtbox(const Transform& transform, const glm::vec3& halfExtents);

	Renderer& renderer;
	btDynamicsWorld* dynamicsWorld;

	static const float attackDistance;
};
