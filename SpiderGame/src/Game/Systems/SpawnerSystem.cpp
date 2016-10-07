
#include "SpawnerSystem.h"

#include "Game/Components/SpawnerComponent.h"
#include "Game/Components/SpiderComponent.h"
#include "Game/Components/PlayerComponent.h"
#include "Game/Components/TransformComponent.h"
#include "Game/Components/CameraComponent.h"

#include "Util.h"

#include <algorithm>

const unsigned SpawnerSystem::maximumSpawnRetries = 10;
const unsigned SpawnerSystem::maxSpiders = 1;

SpawnerSystem::SpawnerSystem(World& world, btDynamicsWorld* dynamicsWorld, std::default_random_engine& generator)
	: System(world), dynamicsWorld(dynamicsWorld), generator(generator)
{
	require<SpawnerComponent>();
}

void SpawnerSystem::updateEntity(float dt, eid_t entity)
{
	SpawnerComponent* component = world.getComponent<SpawnerComponent>(entity);

	if (component->data.candidatePositions.size() <= 0) {
		return;
	}

	component->difficultyRampTimer = (std::min)(component->difficultyRampTimer + dt, component->data.difficultyRampTime);
	component->spawnTimer += dt;

	float currentSpawnTime = component->data.startSpawnTime + component->difficultyRampTimer / component->data.difficultyRampTime * (component->data.endSpawnTime - component->data.startSpawnTime);

	if (component->spawnTimer >= currentSpawnTime) {
		bool spawned = false;

		if (world.getEntitiesWithComponent<SpiderComponent>().size() >= maxSpiders) {
			return;
		}

		std::vector<eid_t> players = world.getEntitiesWithComponent<PlayerComponent>();
		assert(players.size() >= 0);

		PlayerComponent* playerComponent = world.getComponent<PlayerComponent>(players[0]);
		TransformComponent* playerTransformComponent = world.getComponent<TransformComponent>(players[0]);
		CameraComponent* cameraComponent = world.getComponent<CameraComponent>(playerComponent->data.camera);

		std::uniform_int_distribution<int> positionRand(0, component->data.candidatePositions.size()-1);

		for (int i = 0; i < maximumSpawnRetries; i++) {
			glm::vec3 candidatePosition = component->data.candidatePositions[positionRand(generator)];
			glm::vec3 playerPosition = playerTransformComponent->data->getWorldPosition();

			// distance check
			float distance = glm::length(candidatePosition - playerPosition);
			if (distance <= component->data.minSpawnDistance) {
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
			// TODO: Cheating here, the scale is hardcoded to 0.005
			PrefabConstructionInfo info = PrefabConstructionInfo(Transform(candidatePosition, glm::quat(), glm::vec3(0.005f)));
			eid_t spider = world.constructPrefab(component->data.prefab, World::NullEntity, &info);
			spawned = true;
			break;
		}
		
		// If we didn't spawn anything, we'll try again next frame
		if (spawned) {
			component->spawnTimer -= currentSpawnTime;
		}
	}
}
