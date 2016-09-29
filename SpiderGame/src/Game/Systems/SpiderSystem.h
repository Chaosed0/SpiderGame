#pragma once

#include "Framework/System.h"
#include "Framework/CollisionEvent.h"
#include "Renderer/ShaderLoader.h"

#include <glm/glm.hpp>

#include <random>

class Renderer;
class Transform;
class SoundManager;
class btDynamicsWorld;

class SpiderSystem : public System
{
public:
	SpiderSystem(World& world, EventManager& eventManager, btDynamicsWorld* dynamicsWorld, Renderer& renderer, SoundManager& soundManager, std::default_random_engine& generator);
	virtual void updateEntity(float dt, eid_t entity);

	Shader debugShader;
private:
	eid_t createHurtbox(const Transform& transform, const glm::vec3& halfExtents, const std::shared_ptr<Transform>& spiderTransform);
	void onSpiderCollided(const CollisionEvent& collisionEvent);

	Renderer& renderer;
	EventManager& eventManager;
	SoundManager& soundManager;
	btDynamicsWorld* dynamicsWorld;
	std::default_random_engine& generator;

	static const float attackDistance;
};
