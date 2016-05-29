#pragma once

#include <SDL.h>

#include <vector>
#include <memory>

#include "Camera.h"
#include "Shader.h"
#include "Renderer.h"
#include "ModelLoader.h"
#include "Box.h"
#include "Model.h"
#include "Entity.h"

#include "ModelRenderSystem.h"
#include "TestMovementSystem.h"

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
	void draw();

	bool running;
	Uint32 lastUpdate;
	float accumulator;
	float timeDelta;

	SDL_Window* window;
	SDL_GLContext context;

	Camera camera;
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
	std::unique_ptr<TestMovementSystem> testMovementSystem;
};
