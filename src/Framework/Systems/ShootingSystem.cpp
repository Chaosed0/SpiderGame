
#include "ShootingSystem.h"

#include <algorithm>
#include <vector>

#include "Util.h"

#include "Renderer/Renderer.h"
#include "Renderer/Model.h"

#include "Framework/Components/PlayerComponent.h"
#include "Framework/Components/TransformComponent.h"
#include "Framework/Components/RigidbodyMotorComponent.h"

#include "Framework/Components/ModelRenderComponent.h"

ShootingSystem::ShootingSystem(World& world, btDynamicsWorld* dynamicsWorld, Renderer& renderer)
	: System(world),
	dynamicsWorld(dynamicsWorld),
	renderer(renderer),
	lineShader("Shaders/basic.vert", "Shaders/singlecolor.frag")
{
	require<PlayerComponent>();
	require<TransformComponent>();
	require<RigidbodyMotorComponent>();
	requireFinished();
}

void ShootingSystem::updateEntity(float dt, eid_t entity)
{
	RigidbodyMotorComponent* rigidbodyMotorComponent(world.getComponent<RigidbodyMotorComponent>(entity));
	TransformComponent* transformComponent(world.getComponent<TransformComponent>(entity));
	PlayerComponent* playerComponent(world.getComponent<PlayerComponent>(entity));

	playerComponent->shotTimer = std::max(playerComponent->shotTimer + dt, playerComponent->shotCooldown);

	if (playerComponent->shooting &&
		playerComponent->shotTimer >= playerComponent->shotCooldown)
	{
		Transform& transform = transformComponent->transform;
		playerComponent->shotTimer = 0.0f;

		glm::vec3 from = transform.getPosition();
		glm::vec3 to = from - playerComponent->maxShotDistance * glm::vec3(0.0f,0.0f,1.0f) * rigidbodyMotorComponent->facing;
		btVector3 btStart(Util::glmToBt(from));
		btVector3 btEnd(Util::glmToBt(to));
		btCollisionWorld::ClosestRayResultCallback rayCallback(btStart, btEnd);
		this->dynamicsWorld->rayTest(btStart, btEnd, rayCallback);

		Vertex fromVert;
		fromVert.position = from;
		Vertex toVert;
		toVert.position = to;
		Mesh lineMesh(std::vector<Vertex>{fromVert, toVert}, std::vector<unsigned>{0,1}, std::vector<Texture>{});
		lineMesh.material.drawType = GL_LINES;
		lineMesh.material.setProperty("color", MaterialProperty(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)));
		unsigned lineHandle = renderer.getModelHandle(std::vector<Mesh>{ lineMesh });

		eid_t line = world.getNewEntity();
		TransformComponent* transformComponent = world.addComponent<TransformComponent>(line);
		ModelRenderComponent* modelRenderComponent = world.addComponent<ModelRenderComponent>(line);
		modelRenderComponent->rendererHandle = renderer.getRenderableHandle(lineHandle, lineShader);

		if (rayCallback.hasHit()) {
			printf("%p\n", rayCallback.m_collisionObject);
		}
	}
}
