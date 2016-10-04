
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

#include "Framework/Prefab.h"

#include "Util.h"

#include <algorithm>

const float SpiderSpawner::defaultStartSpawnTime = 5.0f;
const float SpiderSpawner::defaultEndSpawnTime = 2.5f;
const float SpiderSpawner::defaultDifficultyRampTime = 30.0f;
const float SpiderSpawner::defaultMinSpawnDistance = 7.0f;
const int SpiderSpawner::maximumSpawnRetries = 10;
const int SpiderSpawner::maxSpiders = 10;

SpiderSpawner::SpiderSpawner(World& world, btDynamicsWorld* dynamicsWorld, Prefab spiderPrefab, Room room, std::default_random_engine& generator)
	: world(world), dynamicsWorld(dynamicsWorld), generator(generator), spiderPrefab(spiderPrefab), room(room),
	startSpawnTime(defaultStartSpawnTime), endSpawnTime(defaultEndSpawnTime),
	spawnTimer(0.0f), difficultyRampTimer(0.0f),
	minSpawnDistance(defaultMinSpawnDistance)
{
	scaleRand = std::uniform_real_distribution<float>(0.004f, 0.006f);
	roomRand = std::uniform_int_distribution<int>(0, room.boxes.size()-1);
}

void SpiderSpawner::update(float dt)
{
	difficultyRampTimer = (std::min)(difficultyRampTimer + dt, difficultyRampTime);
	spawnTimer += dt;

	float currentSpawnTime = startSpawnTime + difficultyRampTimer / difficultyRampTime * (endSpawnTime - startSpawnTime);

	if (spawnTimer >= currentSpawnTime) {
		bool spawned = false;

		if (world.getEntitiesWithComponent<SpiderComponent>().size() >= maxSpiders) {
			return;
		}

		std::vector<eid_t> players = world.getEntitiesWithComponent<PlayerComponent>();
		assert(players.size() >= 0);

		PlayerComponent* playerComponent = world.getComponent<PlayerComponent>(players[0]);
		TransformComponent* playerTransformComponent = world.getComponent<TransformComponent>(players[0]);
		CameraComponent* cameraComponent = world.getComponent<CameraComponent>(playerComponent->data.camera);

		for (int i = 0; i < maximumSpawnRetries; i++) {
			RoomBox box = room.boxes[roomRand(generator)];
			std::uniform_int_distribution<int> xRand(box.left + 1, box.right - 1);
			std::uniform_int_distribution<int> zRand(box.bottom + 1, box.top - 1);

			glm::vec3 candidatePosition(xRand(generator), 0.5f, zRand(generator));
			glm::vec3 playerPosition = playerTransformComponent->data->getWorldPosition();

			// distance check
			float distance = glm::length(candidatePosition - playerPosition);
			if (distance <= minSpawnDistance) {
				continue;
			}

			// Check if we're behind a wall
			eid_t hitEntity = Util::raycast(dynamicsWorld, candidatePosition, playerPosition, CollisionGroupWall);

			// Check if we are visible on camera
			Camera& camera = cameraComponent->data;
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
			spawnTimer -= currentSpawnTime;
		}
	}
}

eid_t SpiderSpawner::makeSpider(const glm::vec3& position)
{
	eid_t spider = world.constructPrefab(spiderPrefab, "Spider");
	CollisionComponent* collisionComponent = world.getComponent<CollisionComponent>(spider);
	collisionComponent->collisionObject->setUserPointer(new eid_t(spider));
	collisionComponent->collisionObject->setWorldTransform(btTransform(btQuaternion::getIdentity(), Util::glmToBt(position)));
	return spider;
}