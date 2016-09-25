
#include "SpiderSpawner.h"

#include "Game/Components/ModelRenderComponent.h"
#include "Game/Components/CollisionComponent.h"
#include "Game/Components/TransformComponent.h"
#include "Game/Components/FollowComponent.h"
#include "Game/Components/RigidbodyMotorComponent.h"
#include "Game/Components/HealthComponent.h"
#include "Game/Components/AudioSourceComponent.h"
#include "Game/Components/SpiderComponent.h"

#include "Game/Components/PlayerComponent.h"
#include "Game/Components/CameraComponent.h"

#include "Util.h"

const float SpiderSpawner::defaultSpawnTime = 3.0f;
const float SpiderSpawner::defaultMinSpawnDistance = 7.0f;
const int SpiderSpawner::maximumSpawnRetries = 10;
const int SpiderSpawner::maxSpiders = 10;

SpiderSpawner::SpiderSpawner(Renderer& renderer, SoundManager& soundManager, World& world, btDynamicsWorld* dynamicsWorld, ModelLoader& modelLoader, std::default_random_engine& generator, Shader& spiderShader, Room room, eid_t player)
	: renderer(renderer), soundManager(soundManager), world(world), dynamicsWorld(dynamicsWorld), generator(generator), spiderShader(spiderShader), player(player), room(room),
	spawnTimer(0.0f), spawnTime(defaultSpawnTime), minSpawnDistance(defaultMinSpawnDistance)
{
	Model spiderModel = modelLoader.loadModelFromPath("assets/models/spider/spider-tex.fbx");
	spiderModelHandle = renderer.getModelHandle(spiderModel);
	scaleRand = std::uniform_real_distribution<float>(0.004f, 0.006f);
	roomRand = std::uniform_int_distribution<int>(0, room.boxes.size()-1);
	spiderSounds = {
		AudioClip("assets/sound/minecraft/spider/say1.ogg"),
		AudioClip("assets/sound/minecraft/spider/say2.ogg"),
		AudioClip("assets/sound/minecraft/spider/say3.ogg"),
		AudioClip("assets/sound/minecraft/spider/say4.ogg")
	};
	spiderDeathSound = AudioClip("assets/sound/minecraft/spider/death.ogg");
}

void SpiderSpawner::update(float dt)
{
	spawnTimer += dt;
	if (spawnTimer >= spawnTime) {
		bool spawned = false;

		if (world.getEntitiesWithComponent<SpiderComponent>().size() >= maxSpiders) {
			return;
		}

		PlayerComponent* playerComponent = world.getComponent<PlayerComponent>(player);
		TransformComponent* playerTransformComponent = world.getComponent<TransformComponent>(player);
		CameraComponent* cameraComponent = world.getComponent<CameraComponent>(playerComponent->camera);

		for (int i = 0; i < maximumSpawnRetries; i++) {
			RoomBox box = room.boxes[roomRand(generator)];
			std::uniform_int_distribution<int> xRand(box.left + 1, box.right - 1);
			std::uniform_int_distribution<int> zRand(box.bottom + 1, box.top - 1);

			glm::vec3 candidatePosition(xRand(generator), 1.0f, zRand(generator));
			glm::vec3 playerPosition = playerTransformComponent->transform->getWorldPosition();

			// distance check
			float distance = glm::length(candidatePosition - playerPosition);
			if (distance <= minSpawnDistance) {
				continue;
			}

			// Check if we're behind a wall
			eid_t hitEntity = Util::raycast(dynamicsWorld, candidatePosition, playerPosition, CollisionGroupWall);

			// Check if we are visible on camera
			Camera& camera = cameraComponent->camera;
			bool inCamera = camera.getFrustum().isInside(candidatePosition);

			if (inCamera && hitEntity == World::NullEntity) {
				// Position is visible, try again
				continue;
			}

			// Far enough away and not able to be seen, do it
			makeSpider(candidatePosition);
			spawned = true;
			break;
		}
		
		// If we didn't spawn anything, we'll try again next frame
		if (spawned) {
			spawnTimer -= spawnTime;
		}
	}
}

eid_t SpiderSpawner::makeSpider(glm::vec3 position)
{
	eid_t spider = world.getNewEntity("Spider");
	ModelRenderComponent* modelComponent = world.addComponent<ModelRenderComponent>(spider);
	TransformComponent* transformComponent = world.addComponent<TransformComponent>(spider);
	CollisionComponent* collisionComponent = world.addComponent<CollisionComponent>(spider);
	FollowComponent* followComponent = world.addComponent<FollowComponent>(spider);
	RigidbodyMotorComponent* rigidbodyMotorComponent = world.addComponent<RigidbodyMotorComponent>(spider);
	HealthComponent* healthComponent = world.addComponent<HealthComponent>(spider);
	AudioSourceComponent* audioSourceComponent = world.addComponent<AudioSourceComponent>(spider);
	SpiderComponent* spiderComponent = world.addComponent<SpiderComponent>(spider);

	TransformComponent* playerTransformComponent = world.getComponent<TransformComponent>(player);

	transformComponent->transform->setPosition(position);
	transformComponent->transform->setScale(glm::vec3(scaleRand(generator)));

	glm::vec3 halfExtents = glm::vec3(200.0f, 75.0f, 120.0f) * transformComponent->transform->getScale().x;
	btCompoundShape* shape = new btCompoundShape();
	btBoxShape* boxShape = new btBoxShape(Util::glmToBt(halfExtents));
	shape->addChildShape(btTransform(btQuaternion::getIdentity(), btVector3(0.0f, halfExtents.y * 1.0f, 0.0f)), boxShape);
	btDefaultMotionState* playerMotionState = new btDefaultMotionState(Util::gameToBt(*transformComponent->transform));
	btRigidBody* spiderRigidBody = new btRigidBody(10.0f, playerMotionState, shape);
	// This pointer is freed by the CollisionComponent destructor
	spiderRigidBody->setUserPointer(new eid_t(spider));
	dynamicsWorld->addRigidBody(spiderRigidBody, CollisionGroupEnemy, CollisionGroupAll);

	collisionComponent->collisionObject = spiderRigidBody;
	collisionComponent->world = dynamicsWorld;

	audioSourceComponent->sourceHandle = soundManager.getSourceHandle();
	soundManager.setSourcePosition(audioSourceComponent->sourceHandle, transformComponent->transform->getWorldPosition());

	followComponent->target = playerTransformComponent;
	followComponent->raycastStartOffset = glm::vec3(0.0f, halfExtents.y, 0.0f);
	rigidbodyMotorComponent->moveSpeed = 3.0f;

	Renderer::RenderableHandle spiderHandle = renderer.getRenderableHandle(spiderModelHandle, spiderShader);
	renderer.setRenderableAnimation(spiderHandle, "AnimStack::idle");
	modelComponent->rendererHandle = spiderHandle;

	healthComponent->health = healthComponent->maxHealth = 100;
	spiderComponent->animState = SPIDER_IDLE;
	spiderComponent->attackTime = 1.0f;
	spiderComponent->sounds = spiderSounds;
	spiderComponent->deathSound = spiderDeathSound;

	return spider;
}