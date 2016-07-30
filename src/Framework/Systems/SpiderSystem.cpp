
#include "SpiderSystem.h"

#include <algorithm>

#include <btBulletDynamicsCommon.h>

#include "Util.h"
#include "Renderer/Box.h"

#include "Framework/Components/TransformComponent.h"
#include "Framework/Components/CollisionComponent.h"
#include "Framework/Components/ModelRenderComponent.h"
#include "Framework/Components/HealthComponent.h"
#include "Framework/Components/SpiderComponent.h"
#include "Framework/Components/FollowComponent.h"
#include "Framework/Components/RigidbodyMotorComponent.h"

#include "Framework/Components/ExpiresComponent.h"
#include "Framework/Components/HurtboxComponent.h"

const float SpiderSystem::attackDistance = 3.0f;

SpiderSystem::SpiderSystem(World& world, btDynamicsWorld* dynamicsWorld, Renderer& renderer)
	: System(world),
	dynamicsWorld(dynamicsWorld),
	renderer(renderer),
	debugShader(nullptr)
{
	require<RigidbodyMotorComponent>();
	require<TransformComponent>();
	require<CollisionComponent>();
	require<ModelRenderComponent>();
	require<HealthComponent>();
	require<FollowComponent>();
	require<SpiderComponent>();
}

void SpiderSystem::updateEntity(float dt, eid_t entity)
{
	RigidbodyMotorComponent* rigidbodyMotorComponent = world.getComponent<RigidbodyMotorComponent>(entity);
	TransformComponent* transformComponent = world.getComponent<TransformComponent>(entity);
	CollisionComponent* collisionComponent = world.getComponent<CollisionComponent>(entity);
	ModelRenderComponent* modelRenderComponent = world.getComponent<ModelRenderComponent>(entity);
	HealthComponent* healthComponent = world.getComponent<HealthComponent>(entity);
	SpiderComponent* spiderComponent = world.getComponent<SpiderComponent>(entity);
	FollowComponent* followComponent = world.getComponent<FollowComponent>(entity);

	assert(collisionComponent->collisionObject->getInternalType() == btCollisionObject::CO_RIGID_BODY);

	btRigidBody* spiderBody = (btRigidBody*)collisionComponent->collisionObject;
	btVector3 velocity = spiderBody->getLinearVelocity();
	SpiderState newState = spiderComponent->animState;
	if (velocity.length() > 0.01f) {
		newState = SPIDER_MOVING;
	} else {
		newState = SPIDER_IDLE;
	}

	float distanceToAttackTarget = glm::length(followComponent->target->transform.getPosition() - transformComponent->transform.getPosition());
	if (spiderComponent->attackTimer > 0.0f) {
		spiderComponent->attackTimer -= dt;
		newState = SPIDER_ATTACKING;
	} else if (distanceToAttackTarget < attackDistance) {
		newState = SPIDER_ATTACKING;
	}

	if (healthComponent->health <= 0) {
		newState = SPIDER_DEAD;
	}

	if (newState == SPIDER_ATTACKING &&
		!spiderComponent->madeHurtbox &&
		spiderComponent->attackTimer < spiderComponent->attackTime / 2.0f)
	{
		btVector3 aabbMin, aabbMax;
		btTransform transform;
		transform.setIdentity();
		spiderBody->getCollisionShape()->getAabb(transform, aabbMin, aabbMax);

		btTransform spiderTransform = spiderBody->getWorldTransform();
		btQuaternion spiderRotation = spiderTransform.getRotation();
		glm::vec3 hurtboxHalfExtents(2.0f, 1.5f, 1.0f);
		btVector3 hurtboxOffset = btVector3(0.0f, 0.0f, aabbMax.z() + hurtboxHalfExtents.z).rotate(spiderRotation.getAxis(), spiderRotation.getAngle());
		Transform hurtboxTransform(Util::btToGlm(spiderTransform.getOrigin() + hurtboxOffset), Util::btToGlm(spiderRotation));
		this->createHurtbox(hurtboxTransform, hurtboxHalfExtents);

		spiderComponent->madeHurtbox = true;
	}

	if (newState != spiderComponent->animState) {
		std::string anim;
		switch(newState) {
		case SPIDER_IDLE:
			anim = "AnimStack::idle";
			rigidbodyMotorComponent->canMove = true;
			break;
		case SPIDER_MOVING:
			anim = "AnimStack::walk";
			rigidbodyMotorComponent->canMove = true;
			break;
		case SPIDER_ATTACKING:
			anim = "AnimStack::attack";
			rigidbodyMotorComponent->canMove = false;
			spiderComponent->madeHurtbox = false;
			spiderComponent->attackTimer = spiderComponent->attackTime;
			break;
		case SPIDER_DEAD:
			anim = "AnimStack::die";
			rigidbodyMotorComponent->canMove = false;
			ExpiresComponent* expiresComponent = world.addComponent<ExpiresComponent>(entity);
			expiresComponent->expiryTime = 2.0f;
			break;
		}

		bool loop = newState != SPIDER_DEAD;
		renderer.setRenderableAnimation(modelRenderComponent->rendererHandle, anim, loop);
		spiderComponent->animState = newState;
	}
}

void SpiderSystem::createHurtbox(const Transform& transform, const glm::vec3& halfExtents)
{
	eid_t hurtboxEntity = world.getNewEntity();
	TransformComponent* transformComponent = world.addComponent<TransformComponent>(hurtboxEntity);
	CollisionComponent* collisionComponent = world.addComponent<CollisionComponent>(hurtboxEntity);
	HurtboxComponent* hurtboxComponent = world.addComponent<HurtboxComponent>(hurtboxEntity);
	ExpiresComponent* expiresComponent = world.addComponent<ExpiresComponent>(hurtboxEntity);

	btBoxShape* shape = new btBoxShape(Util::glmToBt(halfExtents));
	btCollisionObject* collisionObject = new btCollisionObject();
	collisionObject->setUserPointer(new eid_t(hurtboxEntity));
	collisionObject->setCollisionShape(shape);
	collisionObject->setWorldTransform(btTransform(Util::glmToBt(transform.getRotation()), Util::glmToBt(transform.getPosition())));
	collisionObject->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE | btCollisionObject::CF_KINEMATIC_OBJECT);
	dynamicsWorld->addCollisionObject(collisionObject);

	collisionComponent->collisionObject = collisionObject;
	collisionComponent->world = dynamicsWorld;

	expiresComponent->expiryTime = 0.5f;

	if (debugShader != nullptr) {
		ModelRenderComponent* modelComponent = world.addComponent<ModelRenderComponent>(hurtboxEntity);
		Model model(std::vector<Mesh> { getDebugBoxMesh(halfExtents) });
		unsigned debugModelHandle = renderer.getModelHandle(model);
		unsigned renderableHandle = renderer.getRenderableHandle(debugModelHandle, *debugShader);
		modelComponent->rendererHandle = renderableHandle;
		modelComponent->renderer = &this->renderer;
	}
}
