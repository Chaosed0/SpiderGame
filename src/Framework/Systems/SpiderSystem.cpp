
#include "SpiderSystem.h"

#include <algorithm>

#include <btBulletDynamicsCommon.h>

#include "Util.h"

#include "Framework/Components/TransformComponent.h"
#include "Framework/Components/CollisionComponent.h"
#include "Framework/Components/ModelRenderComponent.h"
#include "Framework/Components/HealthComponent.h"
#include "Framework/Components/SpiderComponent.h"
#include "Framework/Components/FollowComponent.h"

#include "Framework/Components/ExpiresComponent.h"

const float SpiderSystem::attackDistance = 3.0f;

SpiderSystem::SpiderSystem(World& world, btDynamicsWorld* dynamicsWorld, Renderer& renderer)
	: System(world),
	dynamicsWorld(dynamicsWorld),
	renderer(renderer)
{
	require<TransformComponent>();
	require<CollisionComponent>();
	require<ModelRenderComponent>();
	require<HealthComponent>();
	require<FollowComponent>();
	require<SpiderComponent>();
}

void SpiderSystem::updateEntity(float dt, eid_t entity)
{
	TransformComponent* transformComponent = world.getComponent<TransformComponent>(entity);
	CollisionComponent* collisionComponent = world.getComponent<CollisionComponent>(entity);
	ModelRenderComponent* modelRenderComponent = world.getComponent<ModelRenderComponent>(entity);
	HealthComponent* healthComponent = world.getComponent<HealthComponent>(entity);
	SpiderComponent* spiderComponent = world.getComponent<SpiderComponent>(entity);
	FollowComponent* followComponent = world.getComponent<FollowComponent>(entity);

	btVector3 velocity = collisionComponent->body->getLinearVelocity();
	SpiderState newState = spiderComponent->animState;
	if (velocity.length() > 0.01f) {
		newState = SPIDER_MOVING;
	} else {
		newState = SPIDER_IDLE;
	}

	float distanceToAttackTarget = glm::length(followComponent->target->transform.getPosition() - transformComponent->transform.getPosition());
	if (spiderComponent->attackTimer < spiderComponent->attackTime) {
		spiderComponent->attackTimer += dt;
		newState = SPIDER_ATTACKING;
	} else if (distanceToAttackTarget < attackDistance) {
		spiderComponent->attackTimer = 0.0f;
		newState = SPIDER_ATTACKING;

		this->createHurtbox(transformComponent->transform);
	}

	if (healthComponent->health <= 0) {
		newState = SPIDER_DEAD;
	}

	if (newState != spiderComponent->animState) {
		std::string anim;
		switch(newState) {
		case SPIDER_IDLE:
			anim = "AnimStack::idle";
			break;
		case SPIDER_MOVING:
			anim = "AnimStack::walk";
			break;
		case SPIDER_ATTACKING:
			anim = "AnimStack::attack";
			break;
		case SPIDER_DEAD:
			anim = "AnimStack::die";
			break;
		}

		if (newState == SPIDER_DEAD) {
			ExpiresComponent* expiresComponent = world.addComponent<ExpiresComponent>(entity);
			expiresComponent->expiryTime = 2.0f;
		}

		bool loop = newState != SPIDER_DEAD;
		renderer.setRenderableAnimation(modelRenderComponent->rendererHandle, anim, loop);
		spiderComponent->animState = newState;
	}
}

void SpiderSystem::createHurtbox(const Transform& transform)
{
	eid_t hurtboxEntity = world.getNewEntity();
	CollisionComponent* collisionComponent = world.addComponent<CollisionComponent>(hurtboxEntity);
	ExpiresComponent* expiresComponent = world.addComponent<ExpiresComponent>(hurtboxEntity);

	glm::vec3 hurtboxCenter = transform.getPosition() + glm::vec3(0.0f, 0.0f, -100.0f) * transform.getScale() * transform.getRotation();
	glm::vec3 hurtboxHalfExtents(10.0f, 10.0f, 5.0f);
	btBoxShape* shape = new btBoxShape(Util::glmToBt(hurtboxHalfExtents));
	btRigidBody* rigidBody = new btRigidBody(0.0f, new btDefaultMotionState(), shape);
	rigidBody->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
	rigidBody->setUserPointer(new eid_t(hurtboxEntity));
	dynamicsWorld->addCollisionObject(rigidBody);

	collisionComponent->body = rigidBody;
	collisionComponent->world = dynamicsWorld;

	expiresComponent->expiryTime = 0.5f;
}
