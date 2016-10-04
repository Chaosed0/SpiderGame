
#include "SpiderSystem.h"

#include <algorithm>

#include <btBulletDynamicsCommon.h>

#include "Util.h"
#include "Renderer/Box.h"
#include "Renderer/Renderer.h"

#include "Game/Components/TransformComponent.h"
#include "Game/Components/CollisionComponent.h"
#include "Game/Components/ModelRenderComponent.h"
#include "Game/Components/HealthComponent.h"
#include "Game/Components/SpiderComponent.h"
#include "Game/Components/FollowComponent.h"
#include "Game/Components/RigidbodyMotorComponent.h"
#include "Game/Components/AudioSourceComponent.h"

#include "Game/Components/ExpiresComponent.h"
#include "Game/Components/HurtboxComponent.h"

#include "Sound/SoundManager.h"

const float SpiderSystem::attackDistance = 6.0f;
const float SpiderSystem::leadTime = 0.25f;

SpiderSystem::SpiderSystem(World& world, EventManager& eventManager, btDynamicsWorld* dynamicsWorld, Renderer& renderer, SoundManager& soundManager, std::default_random_engine& generator)
	: System(world),
	dynamicsWorld(dynamicsWorld),
	eventManager(eventManager),
	renderer(renderer),
	soundManager(soundManager),
	generator(generator)
{
	require<RigidbodyMotorComponent>();
	require<TransformComponent>();
	require<CollisionComponent>();
	require<ModelRenderComponent>();
	require<HealthComponent>();
	require<FollowComponent>();
	require<AudioSourceComponent>();
	require<SpiderComponent>();

	eventManager.registerForEvent<CollisionEvent>(std::bind(&SpiderSystem::onSpiderCollided, this, std::placeholders::_1));
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
	AudioSourceComponent* audioSourceComponent = world.getComponent<AudioSourceComponent>(entity);

	assert(collisionComponent->collisionObject->getInternalType() == btCollisionObject::CO_RIGID_BODY);

	btRigidBody* spiderBody = (btRigidBody*)collisionComponent->collisionObject;
	btVector3 velocity = spiderBody->getLinearVelocity();
	SpiderState newState = spiderComponent->animState;

	std::shared_ptr<Transform> targetTransform = world.getComponent<TransformComponent>(followComponent->data.target)->data;
	
	float distanceToAttackTarget = glm::length(targetTransform->getWorldPosition() - transformComponent->data->getWorldPosition());

	switch (spiderComponent->animState) {
	case SPIDER_IDLE:
	case SPIDER_MOVING:
		if (distanceToAttackTarget < attackDistance) {
			newState = SPIDER_PREPARING_LEAP;
		} else if (velocity.length() > 0.01f) {
			newState = SPIDER_MOVING;
		} else {
			newState = SPIDER_IDLE;
		}
		break;
	case SPIDER_PREPARING_LEAP: {
		followComponent->enabled = false;

		CollisionComponent* targetCollisionComponent = world.getComponent<CollisionComponent>(followComponent->data.target);
		glm::vec3 leadVec(0.0f);
		if (targetCollisionComponent != nullptr) {
			btCollisionObject* collisionObject = targetCollisionComponent->collisionObject;
			if (collisionObject->getInternalType() == btCollisionObject::CO_RIGID_BODY) {
				btRigidBody* targetBody = (btRigidBody*)targetCollisionComponent->collisionObject;
				leadVec = Util::btToGlm(targetBody->getLinearVelocity()) * leadTime;
			}
		}

		glm::vec3 leapTarget = targetTransform->getWorldPosition() + leadVec;
		glm::vec3 leapDir = leapTarget - transformComponent->data->getWorldPosition();
		float angle = atan2f(leapDir.x, leapDir.z);
		glm::quat facing(glm::angleAxis(angle, glm::vec3(0.0f, 1.0f, 0.0f)));
		rigidbodyMotorComponent->movement = glm::vec2(0.0f, 0.0f);
		rigidbodyMotorComponent->facing = facing;

		spiderComponent->timer += dt;
		if (spiderComponent->timer >= spiderComponent->data.attackTime) {
			newState = SPIDER_LEAPING;
			rigidbodyMotorComponent->movement = glm::vec2(-1.0f, 0.0f);
			rigidbodyMotorComponent->data.moveSpeed = spiderComponent->data.leapMoveSpeed;
			rigidbodyMotorComponent->canJump = true;
			rigidbodyMotorComponent->jump = true;

			btVector3 aabbMin, aabbMax;
			btTransform transform;
			transform.setIdentity();
			spiderBody->getCollisionShape()->getAabb(transform, aabbMin, aabbMax);

			// Make the hurtbox the size of the front of the spider, and make it jut out a little bit
			// so the player is only hurt from the front
			glm::vec3 hurtboxHalfExtents((aabbMax.x() - aabbMin.x()) / 2.0f, (aabbMax.y() - aabbMin.y()) / 2.0f, 0.1f);
			glm::vec3 hurtboxOffset(glm::vec3(0.0f, (aabbMax.y() - aabbMin.y()) / 2.0f, aabbMax.z() + hurtboxHalfExtents.z) * (1.0f / transformComponent->data->getScale()));
			Transform hurtboxTransform(hurtboxOffset);

			spiderComponent->hurtbox = this->createHurtbox(hurtboxTransform, hurtboxHalfExtents, transformComponent->data);
			spiderComponent->soundTimer = spiderComponent->soundTime;
			spiderComponent->timer = 0.0f;
		}
		break;
	}
	case SPIDER_LEAPING:
		// Transition controlled by onSpiderCollided
		break;
	case SPIDER_LEAP_RECOVERY:
		rigidbodyMotorComponent->movement = glm::vec2(0.0f, 0.0f);
		spiderComponent->timer += dt;
		if (spiderComponent->timer >= spiderComponent->data.recoveryTime) {
			spiderComponent->timer = 0.0f;
			followComponent->enabled = true;
			newState = SPIDER_IDLE;
		}
		break;
	default:
		break;
	}

	// This transition can happen at any time
	if (healthComponent->data.health <= 0) {
		newState = SPIDER_DEAD;
		
		if (spiderComponent->hurtbox != World::NullEntity) {
			world.removeEntity(spiderComponent->hurtbox);
		}
	}

	spiderComponent->soundTimer += dt;
	if (newState != SPIDER_DEAD &&
		spiderComponent->soundTime > 0 &&
		spiderComponent->data.sounds.size() > 0 &&
		spiderComponent->soundTimer >= spiderComponent->soundTime)
	{
		spiderComponent->soundTimer = 0.0f;
		std::uniform_int_distribution<int> soundRand(0, spiderComponent->data.sounds.size() - 1);
		AudioClip& clip = spiderComponent->data.sounds[soundRand(generator)];
		soundManager.playClipAtSource(clip, audioSourceComponent->sourceHandle);
		spiderComponent->soundTime = -1.0f;
	}

	// Handles the case where it isn't initialized too
	if (spiderComponent->soundTime < 0.0f) {
		std::uniform_real_distribution<float> nextSoundTime(spiderComponent->data.soundTimeMin, spiderComponent->data.soundTimeMax);
		spiderComponent->soundTime = nextSoundTime(generator);
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
		case SPIDER_PREPARING_LEAP:
			anim = "AnimStack::attack";
			break;
		case SPIDER_LEAPING:
			break;
		case SPIDER_LEAP_RECOVERY:
			break;
		case SPIDER_DEAD: {
			anim = "AnimStack::die";
			rigidbodyMotorComponent->canMove = false;
			ExpiresComponent* expiresComponent = world.addComponent<ExpiresComponent>(entity);
			expiresComponent->data.expiryTime = 2.0f;
			soundManager.playClipAtSource(spiderComponent->data.deathSound, audioSourceComponent->sourceHandle);
			break;
		}
		default:
			break;
		}

		if (!anim.empty()) {
			bool loop = newState != SPIDER_DEAD && newState != SPIDER_PREPARING_LEAP;
			renderer.setRenderableAnimation(modelRenderComponent->rendererHandle, anim, loop);
		}
		spiderComponent->animState = newState;
	}
}

eid_t SpiderSystem::createHurtbox(const Transform& transform, const glm::vec3& halfExtents, const std::shared_ptr<Transform>& spiderTransform)
{
	eid_t hurtboxEntity = world.getNewEntity("Hurtbox");
	TransformComponent* transformComponent = world.addComponent<TransformComponent>(hurtboxEntity);
	CollisionComponent* collisionComponent = world.addComponent<CollisionComponent>(hurtboxEntity);
	HurtboxComponent* hurtboxComponent = world.addComponent<HurtboxComponent>(hurtboxEntity);

	btBoxShape* shape = new btBoxShape(Util::glmToBt(halfExtents));
	btCollisionObject* collisionObject = new btCollisionObject();
	collisionObject->setUserPointer(new eid_t(hurtboxEntity));
	collisionObject->setCollisionShape(shape);
	collisionObject->setWorldTransform(Util::gameToBt(transform));
	collisionObject->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE | btCollisionObject::CF_KINEMATIC_OBJECT);
	dynamicsWorld->addCollisionObject(collisionObject, CollisionGroupHurtbox, CollisionGroupPlayer);

	collisionComponent->collisionObject = collisionObject;
	collisionComponent->world = dynamicsWorld;
	collisionComponent->controlsMovement = false;

	transformComponent->data->setPosition(transform.getPosition());
	transformComponent->data->setRotation(transform.getRotation());
	transformComponent->data->setScale(transform.getScale());
	transformComponent->data->setParent(spiderTransform);

	if (debugShader.isValid()) {
		ModelRenderComponent* modelComponent = world.addComponent<ModelRenderComponent>(hurtboxEntity);
		Model model(getDebugBoxMesh(halfExtents));
		Renderer::ModelHandle debugModelHandle = renderer.getModelHandle(model);
		Renderer::RenderableHandle renderableHandle = renderer.getRenderableHandle(debugModelHandle, debugShader);
		modelComponent->rendererHandle = renderableHandle;
	}

	return hurtboxEntity;
}

void SpiderSystem::onSpiderCollided(const CollisionEvent& collisionEvent)
{
	if (collisionEvent.type != CollisionResponseType_Began) {
		return;
	}

	eid_t spider = collisionEvent.e1;
	eid_t otherEntity = collisionEvent.e2;
	bool hasComponents = world.orderEntities(spider, otherEntity, requiredComponents, ComponentBitmask());
	if (!hasComponents) {
		return;
	}

	SpiderComponent* spiderComponent = world.getComponent<SpiderComponent>(spider);
	if (spiderComponent->animState != SPIDER_LEAPING) {
		return;
	}

	RigidbodyMotorComponent* motorComponent = world.getComponent<RigidbodyMotorComponent>(spider);
	motorComponent->data.moveSpeed = spiderComponent->data.normalMoveSpeed;
	spiderComponent->animState = SPIDER_LEAP_RECOVERY;
	spiderComponent->timer = 0.0f;

	world.removeEntity(spiderComponent->hurtbox);
	spiderComponent->hurtbox = World::NullEntity;
}
