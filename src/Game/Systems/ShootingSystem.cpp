
#include "ShootingSystem.h"

#include <algorithm>
#include <vector>

#include "Util.h"

#include "Renderer/Renderer.h"
#include "Renderer/Model.h"
#include "Renderer/Shader.h"

#include "Game/Components/PlayerComponent.h"
#include "Game/Components/TransformComponent.h"
#include "Game/Components/RigidbodyMotorComponent.h"
#include "Game/Components/CollisionComponent.h"
#include "Game/Components/HealthComponent.h"

#include "Game/Components/ExpiresComponent.h"
#include "Game/Components/ModelRenderComponent.h"
#include "Game/Components/VelocityComponent.h"

ShootingSystem::ShootingSystem(World& world, btDynamicsWorld* dynamicsWorld, Renderer& renderer)
	: System(world),
	dynamicsWorld(dynamicsWorld),
	renderer(renderer),
	lineShader("Shaders/basic.vert", "Shaders/singlecolor.frag")
{
	require<PlayerComponent>();
	require<TransformComponent>();
	require<RigidbodyMotorComponent>();

	Vertex fromVert;
	fromVert.position = glm::vec3(0.0f);
	Vertex toVert;
	toVert.position = Util::forward * 3.0f;
	Mesh lineMesh(std::vector<Vertex>{fromVert, toVert}, std::vector<unsigned>{0,1}, std::vector<Texture>{});
	lineMesh.material.drawType = GL_LINES;
	lineMesh.material.setProperty("color", MaterialProperty(glm::vec4(0.5f, 0.5f, 0.0f, 1.0f)));
	bulletMeshHandle = renderer.getModelHandle(std::vector<Mesh>{ lineMesh });
}

void ShootingSystem::updateEntity(float dt, eid_t entity)
{
	RigidbodyMotorComponent* rigidbodyMotorComponent(world.getComponent<RigidbodyMotorComponent>(entity));
	TransformComponent* transformComponent(world.getComponent<TransformComponent>(entity));
	PlayerComponent* playerComponent(world.getComponent<PlayerComponent>(entity));

	playerComponent->shotTimer = std::min(playerComponent->shotTimer + dt, playerComponent->shotCooldown);

	if (playerComponent->shooting &&
		playerComponent->shotTimer >= playerComponent->shotCooldown)
	{
		Transform& transform = transformComponent->transform;
		playerComponent->shotTimer = 0.0f;

		TransformComponent* cameraTransformComponent = world.getComponent<TransformComponent>(playerComponent->camera);

		glm::vec3 from = cameraTransformComponent->transform.getPosition();
		glm::vec3 to = from + cameraTransformComponent->transform.getRotation() * (Util::forward * playerComponent->maxShotDistance);
		btVector3 btStart(Util::glmToBt(from));
		btVector3 btEnd(Util::glmToBt(to));
		btCollisionWorld::ClosestRayResultCallback rayCallback(btStart, btEnd);
		this->dynamicsWorld->rayTest(btStart, btEnd, rayCallback);

		glLineWidth(3.0f);

		eid_t line = world.getNewEntity();
		TransformComponent* transformComponent = world.addComponent<TransformComponent>(line);
		ModelRenderComponent* modelRenderComponent = world.addComponent<ModelRenderComponent>(line);
		ExpiresComponent* expiresComponent = world.addComponent<ExpiresComponent>(line);
		VelocityComponent* velocityComponent = world.addComponent<VelocityComponent>(line);

		transformComponent->transform.setPosition(cameraTransformComponent->transform.getPosition() + cameraTransformComponent->transform.getRotation() * Util::right * 0.1f);
		transformComponent->transform.setRotation(cameraTransformComponent->transform.getRotation());
		modelRenderComponent->rendererHandle = renderer.getRenderableHandle(bulletMeshHandle, lineShader);
		modelRenderComponent->renderer = &renderer;
		expiresComponent->expiryTime = 0.2f;
		velocityComponent->speed = 100.0f;

		if (!rayCallback.hasHit()) {
			return;
		}

		void* userPtr = rayCallback.m_collisionObject->getUserPointer();
		if (userPtr == nullptr) {
			return;
		}

		eid_t hitEntity = *((eid_t*)userPtr);
		HealthComponent* enemyHealthComponent = world.getComponent<HealthComponent>(hitEntity);
		if (enemyHealthComponent == nullptr) {
			return;
		}

		enemyHealthComponent->health -= playerComponent->shotDamage;
		printf("%s %d\n", world.getEntityName(hitEntity).c_str(), enemyHealthComponent->health);
	}
}
