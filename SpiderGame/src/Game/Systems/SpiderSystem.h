#pragma once

#include "Framework/System.h"
#include "Renderer/Shader.h"

#include <glm/glm.hpp>

#include <random>

class Renderer;
class Transform;
class SoundManager;
class btDynamicsWorld;

class SpiderSystem : public System
{
public:
	SpiderSystem(World& world, btDynamicsWorld* dynamicsWorld, Renderer& renderer, SoundManager& soundManager, std::default_random_engine& generator);
	virtual void updateEntity(float dt, eid_t entity);

	Shader debugShader;
private:
	void createHurtbox(const Transform& transform, const glm::vec3& halfExtents);

	Renderer& renderer;
	SoundManager& soundManager;
	btDynamicsWorld* dynamicsWorld;
	std::default_random_engine& generator;

	static const float attackDistance;
};
