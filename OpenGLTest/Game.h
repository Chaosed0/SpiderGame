#pragma once

#include <SDL.h>
#include <btBulletDynamicsCommon.h>

#include <vector>
#include <memory>

#include "Camera.h"
#include "Shader.h"
#include "Renderer.h"
#include "ModelLoader.h"
#include "Box.h"
#include "Model.h"
#include "Entity.h"
#include "Console.h"

#include "ModelRenderSystem.h"
#include "CollisionUpdateSystem.h"

class Game
{
public:
	Game();
	int run();
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

	std::shared_ptr<Camera> camera;
	Renderer renderer;
	ModelLoader modelLoader;

	Shader shader;
	Shader lightShader;
	Shader skyboxShader;

	std::vector<Transform> pointLightTransforms;
	Model pointLightModel;
	Model skyboxModel;

	std::vector<Entity> entities;
	std::unique_ptr<ModelRenderSystem> modelRenderSystem;
	std::unique_ptr<CollisionUpdateSystem> collisionUpdateSystem;

	std::unique_ptr<btDiscreteDynamicsWorld> dynamicsWorld;

	std::unique_ptr<Console> console;
	bool consoleIsVisible;

	void exit();
	void setWireframe(bool on);
	bool wireframe;
};
