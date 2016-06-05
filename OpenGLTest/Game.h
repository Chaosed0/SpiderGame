#pragma once

#include <SDL.h>
#include <btBulletDynamicsCommon.h>

#include <vector>
#include <memory>

#include "Renderer/Shader.h"
#include "Renderer/Renderer.h"
#include "Renderer/ModelLoader.h"
#include "Renderer/Box.h"
#include "Renderer/Model.h"
#include "Framework/Entity.h"
#include "Console/Console.h"

#include "Framework/Systems/ModelRenderSystem.h"
#include "Framework/Systems/CollisionUpdateSystem.h"
#include "Framework/Systems/CameraSystem.h"

#include "Framework/Components/TransformComponent.h"

class Game
{
public:
	Game();
	int run();

	// Shouldn't be called outside of the physics timestep
	void fixedUpdate(btDynamicsWorld* world, float dt);
private:
	int setup();
	int loop();
	int teardown();

	void update();

	void handleEvent(SDL_Event& event);
	void draw();

	bool running;
	Uint32 lastUpdate;
	float accumulator;
	float timeDelta;

	SDL_Window* window;
	SDL_GLContext context;

	Renderer renderer;
	ModelLoader modelLoader;

	Shader shader;
	Shader lightShader;
	Shader skyboxShader;

	std::vector<Transform> pointLightTransforms;
	Model pointLightModel;
	Model skyboxModel;

	Entity player;
	btRigidBody* playerBody;
	btRigidBody* floorBody;
	std::shared_ptr<TransformComponent> cameraTransformComponent;

	std::vector<Entity> entities;
	std::unique_ptr<ModelRenderSystem> modelRenderSystem;
	std::unique_ptr<CollisionUpdateSystem> collisionUpdateSystem;
	std::unique_ptr<CameraSystem> cameraSystem;

	btDiscreteDynamicsWorld* dynamicsWorld;

	std::unique_ptr<Console> console;
	bool consoleIsVisible;

	void exit();
	void setWireframe(bool on);
	bool wireframe;
};
