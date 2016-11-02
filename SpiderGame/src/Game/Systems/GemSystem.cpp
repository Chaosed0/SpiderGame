
#include "GemSystem.h"

#include "Game/Components/TransformComponent.h"
#include "Game/Components/GemComponent.h"
#include "Game/Components/PointlightComponent.h"

#include "Game/Components/PlayerComponent.h"
#include "Game/Components/CollisionComponent.h"
#include "Game/Components/VelocityComponent.h"

#include "Game/Systems/GameEndingSystem.h"

#include <algorithm>

const float GemSystem::endGemHeight = 2.0f;
const float GemSystem::endGemAngularSpeed = 0.25f;
const float GemSystem::airLiftTime = 8.0f;

GemSystem::GemSystem(World& world, Renderer& renderer, EventManager& eventManager)
	: System(world),
	renderer(renderer),
	allGemsPlaced(false),
	eventManager(eventManager)
{
	require<GemComponent>();
	require<TransformComponent>();
	require<CollisionComponent>();
	require<VelocityComponent>();

	eventManager.registerForEvent<AllGemsCollectedEvent>(std::bind(&GemSystem::onAllGemsCollected, this, std::placeholders::_1));
}

void GemSystem::updateEntity(float dt, eid_t entity)
{
	GemComponent* gemComponent = world.getComponent<GemComponent>(entity);

	if (gemComponent->data.state == GemState_Free) {
		return;
	}

	// Always check lightState before dereferencing this
	PointLightComponent* pointLightComponent = world.getComponent<PointLightComponent>(gemComponent->light);

	if (gemComponent->data.lightState == GemLightState_Unset) {
		PointLight light = renderer.getPointLight(pointLightComponent->handle);
		renderer.setPointLight(pointLightComponent->handle, gemComponent->data.minIntensity);
		gemComponent->data.lightState = GemLightState_Small;
	}

	std::vector<eid_t> players = world.getEntitiesWithComponent<PlayerComponent>();
	assert(players.size());
	PlayerComponent* playerComponent = world.getComponent<PlayerComponent>(players[0]);

	if (allGemsPlaced) {
		TransformComponent* gemTransformComponent = world.getComponent<TransformComponent>(entity);
		glm::vec3 currentGemPosition = gemTransformComponent->data->getWorldPosition();

		// Turn off collision
		CollisionComponent* collisionComponent = world.getComponent<CollisionComponent>(entity);
		int flags = collisionComponent->collisionObject->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE;
		collisionComponent->collisionObject->setCollisionFlags(flags);

		float heightChange = 0.0f;
		if (gemComponent->endGameTimer < this->airLiftTime) {
			heightChange = dt / this->airLiftTime * this->endGemHeight;
		}

		// Assume the gems circle around the origin
		float angularSpeed = (std::min)(gemComponent->endGameTimer / playerComponent->data.gemDefenseTime, 1.0f) * this->endGemAngularSpeed;
		float theta = atan2f(currentGemPosition.x, currentGemPosition.z) + angularSpeed;
		float rad = glm::distance(currentGemPosition, glm::vec3(0.0f, currentGemPosition.y, 0.0f));

		// Stagger each gem turn on
		float index = gemComponent->data.color + 0.5f;
		float currentIndex = playerComponent->gameEndTimer / playerComponent->data.gemDefenseTime * GemColor_Unknown;

		// Figure out when to "turn on" the light
		if (currentIndex >= index && gemComponent->data.lightState == GemLightState_Small) {
			PointLight light = renderer.getPointLight(pointLightComponent->handle);
			renderer.setPointLight(pointLightComponent->handle, gemComponent->data.maxIntensity);
			gemComponent->data.lightState = GemLightState_Max;

			GemLightOnEvent lightOnEvent;
			lightOnEvent.source = entity;
			eventManager.sendEvent(lightOnEvent);
		}

		glm::vec3 newPosition(sin(theta) * rad, currentGemPosition.y + heightChange, cos(theta) * rad);
		gemTransformComponent->data->setPosition(newPosition);

		gemComponent->endGameTimer += dt;
	}

	if (gemComponent->data.state == GemState_ShouldFree) {
		// Turn collision back on
		CollisionComponent* collisionComponent = world.getComponent<CollisionComponent>(entity);
		int flags = collisionComponent->collisionObject->getCollisionFlags() & (~btCollisionObject::CF_NO_CONTACT_RESPONSE);
		collisionComponent->collisionObject->setCollisionFlags(flags);

		// Remove velocity and give control back to physics
		world.removeComponent<VelocityComponent>(entity);
		collisionComponent->controlsMovement = true;
		btVector3 inertia(((btRigidBody*)collisionComponent->collisionObject)->getLocalInertia());
		((btRigidBody*)collisionComponent->collisionObject)->setMassProps(1.0f, inertia);

		// Delete light
		world.removeEntity(gemComponent->light);
		gemComponent->data.lightState = GemLightState_Deleted;

		gemComponent->data.state = GemState_Free;
	}
}

void GemSystem::onAllGemsCollected(const AllGemsCollectedEvent& allGemsCollectedEvent)
{
	this->allGemsPlaced = true;

	std::vector<eid_t> gems = world.getEntitiesWithComponent<GemComponent>();
	for (unsigned i = 0; i < gems.size(); ++i) {
		world.removeComponent<CollisionComponent>(gems[i]);
	}
}
