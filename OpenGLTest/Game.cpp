
#include "Game.h"

#include <SDL.h>
#include <SDL_Image.h>

#include <cstdio>
#include <cstdint>
#include <sstream>
#include <random>
#include <ctime>

#include <Windows.h>
#include <gl\glew.h>
#include <gl\GL.h>
#include <gl\GLU.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include "Renderer/Camera.h"
#include "Util.h"

#include "Framework/Components/ModelRenderComponent.h"
#include "Framework/Components/CollisionComponent.h"
#include "Framework/Components/CameraComponent.h"

const static int updatesPerSecond = 60;
const static int windowWidth = 1280;
const static int windowHeight = 1024;

static float cameraHorizontal = 0.0f;
static float cameraVertical = 0.0f;
static glm::vec3 movement(0,0,0);

Game::Game()
{
	running = false;
	wireframe = false;
	lastUpdate = UINT32_MAX;
	consoleIsVisible = false;
}

int Game::run()
{
	if (setup() < 0) {
		return -1;
	}

	running = true;
	loop();

	teardown();

	return 0;
}

void Game::exit()
{
	running = false;
}

void Game::setWireframe(bool on)
{
	wireframe = on;
}

int Game::setup()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf ("SDL could not initialize, error: %s\n", SDL_GetError());
		return -1;
	}

	if (IMG_Init(IMG_INIT_PNG) < 0)
	{
		printf ("SDL_Image could not initialize, error: %s\n", IMG_GetError());
		return -1;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

	window = SDL_CreateWindow("window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

	if (window == NULL)
	{
		printf ("Could not create window, error: %s\n", SDL_GetError());
		return -1;
	}

	context = SDL_GL_CreateContext(window);

	if (context == NULL)
	{
		printf("Could not create OpenGL context, error: %s\n", SDL_GetError());
		return -1;
	}

	if (!renderer.initialize()) {
		return -1;
	}

	SDL_SetRelativeMouseMode(SDL_TRUE);

	/* Console */
	console = std::make_unique<Console>((float)windowWidth, windowHeight * 0.6f, (float)windowWidth, (float)windowHeight);
	console->addCallback("exit", CallbackMap::defineCallback(std::bind(&Game::exit, this)));
	console->addCallback("wireframe", CallbackMap::defineCallback<bool>(std::bind(&Game::setWireframe, this, std::placeholders::_1)));

	// Initialize renderer debugging output
	renderer.setDebugLogCallback(std::bind(&Console::print, this->console.get(), std::placeholders::_1));

	/* Physics */
	btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
	btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
	btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;
	dynamicsWorld = std::make_unique<btDiscreteDynamicsWorld>(dispatcher, overlappingPairCache, solver, collisionConfiguration);
	dynamicsWorld->setGravity(btVector3(0.0f, -10.0f, 0.0f));

	btStaticPlaneShape* planeShape = new btStaticPlaneShape(btVector3(0.0f, 1.0f, 0.0f), 0.0f);
	btDefaultMotionState* motionState = new btDefaultMotionState(btTransform::getIdentity());
	btRigidBody* body = new btRigidBody(0.0f, motionState, planeShape, btVector3(0.0f, 0.0f, 0.0f));
	dynamicsWorld->addRigidBody(body);

	/* Scene */
	lightShader.compileAndLink("Shaders/basic.vert", "Shaders/white.frag");
	shader.compileAndLink("Shaders/basic.vert", "Shaders/lightcolor.frag");
	skyboxShader.compileAndLink("Shaders/skybox.vert", "Shaders/skybox.frag");

	glm::vec3 pointLightPositions[] = {
		glm::vec3(0.7f,  0.2f,  2.0f),
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(-4.0f,  2.0f, -12.0f),
		glm::vec3(0.0f,  0.0f, -3.0f)
	};

	pointLightTransforms.resize(4);
	for (int i = 0; i < 4; i++) {
		PointLight light;
		light.constant = 1.0f;
		light.linear = 0.09f;
		light.quadratic = 0.032f;
		light.ambient = glm::vec3(0.2f);
		light.diffuse = glm::vec3(0.5f);
		light.specular = glm::vec3(1.0f);
		renderer.setPointLight(i, light);

		pointLightTransforms[i].setPosition(pointLightPositions[i]);
		pointLightTransforms[i].setScale(glm::vec3(0.2f));
	}

	DirLight dirLight;
	dirLight.direction = glm::vec3(-0.2f, -1.0f, -0.3f);
	dirLight.ambient = glm::vec3(0.2f);
	dirLight.diffuse = glm::vec3(0.5f);
	dirLight.specular = glm::vec3(1.0f);
	renderer.setDirLight(dirLight);

	modelLoader.assignModelToId("pointLight", std::vector<Mesh> { getBox(std::vector<Texture>{}) });

	std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("assets/img/skybox/miramar_ft.tga");
	skyboxFaces.push_back("assets/img/skybox/miramar_bk.tga");
	skyboxFaces.push_back("assets/img/skybox/miramar_up.tga");
	skyboxFaces.push_back("assets/img/skybox/miramar_dn.tga");
	skyboxFaces.push_back("assets/img/skybox/miramar_rt.tga");
	skyboxFaces.push_back("assets/img/skybox/miramar_lf.tga");
	modelLoader.assignModelToId("skybox", std::vector<Mesh> { getSkybox(skyboxFaces) });

	pointLightModel = modelLoader.loadModelById("pointLight");
	skyboxModel = modelLoader.loadModelById("skybox");

	Model shroomModel = modelLoader.loadModelFromPath("assets/models/shroom/shroom.obj");

	std::default_random_engine generator;
	generator.seed((unsigned int)time(NULL));
	std::uniform_real_distribution<float> positionRand(-5.0f, 5.0f);
	std::uniform_real_distribution<float> angleRand(-glm::pi<float>(), glm::pi<float>());
	std::uniform_real_distribution<float> axisRand(-1.0f, 1.0f);
	std::uniform_real_distribution<float> scaleRand(0.5f, 2.0f);

	for (int i = 0; i < 10; i++) {
		Entity shroom;
		std::shared_ptr<ModelRenderComponent> modelComponent = shroom.addComponent<ModelRenderComponent>();
		std::shared_ptr<TransformComponent> transformComponent = shroom.addComponent<TransformComponent>();
		std::shared_ptr<CollisionComponent> collisionComponent = shroom.addComponent<CollisionComponent>();

		float z = axisRand(generator);
		float axisAngle = angleRand(generator);
		float angle = angleRand(generator);
		transformComponent->transform.setPosition(glm::vec3(positionRand(generator), positionRand(generator) + 5.0f, positionRand(generator)));
		transformComponent->transform.setRotation(glm::angleAxis(angle, glm::vec3(sqrt(1 - z*z) * cos(axisAngle), sqrt(1 - z*z) * sin(axisAngle), z)));
		transformComponent->transform.setScale(glm::vec3(scaleRand(generator)));

		btSphereShape* shape = new btSphereShape(0.5f * transformComponent->transform.getScale().x);
		btDefaultMotionState* motionState = new btDefaultMotionState(Util::transformToBt(transformComponent->transform));
		collisionComponent->body = std::shared_ptr<btRigidBody>(new btRigidBody(1.0f, motionState, shape, btVector3(0.0f, 0.0f, 0.0f)));
		dynamicsWorld->addRigidBody(collisionComponent->body.get());

		unsigned int shroomHandle = renderer.getHandle(shroomModel, shader);
		modelComponent->rendererHandle = shroomHandle;
		entities.push_back(shroom);
	}

	std::shared_ptr<CameraComponent> cameraComponent = player.addComponent<CameraComponent>();
	cameraComponent->camera = Camera(glm::radians(90.0f), windowWidth, windowHeight, 0.1f, 1000000.0f);
	playerTransform = player.addComponent<TransformComponent>();
	playerTransform->transform.setPosition(glm::vec3(0.0f, 0.0f, 10.0f));
	entities.push_back(player);
	renderer.setCamera(&cameraComponent->camera);

	unsigned int skyboxHandle = renderer.getHandle(skyboxModel, skyboxShader);
	renderer.updateTransform(skyboxHandle, Transform::identity);
	for (unsigned int i = 0; i < pointLightTransforms.size(); i++) {
		unsigned int lightHandle = renderer.getHandle(pointLightModel, lightShader);
		renderer.updateTransform(lightHandle, pointLightTransforms[i]);
	}

	modelRenderSystem = std::make_unique<ModelRenderSystem>(renderer);
	collisionUpdateSystem = std::make_unique<CollisionUpdateSystem>();
	cameraSystem = std::make_unique<CameraSystem>(renderer);

	return 0;
}

int Game::teardown()
{
	return 0;
}

int Game::loop()
{
	while (running)
	{
		accumulator += SDL_GetTicks() - lastUpdate;
		lastUpdate = SDL_GetTicks();
		if (accumulator >= 1.0f / updatesPerSecond)
		{
			timeDelta = 1.0f / updatesPerSecond;
			update();
			accumulator -= 1.0f / updatesPerSecond;
		}
	
		draw();
	}

	return 0;
}

void Game::draw()
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, this->wireframe ? GL_LINE : GL_FILL);

	renderer.draw();

	if (consoleIsVisible) {
		glDisable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		console->draw();
	}

	SDL_GL_SwapWindow(window);
}

void Game::update()
{
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		this->handleEvent(event);
	}
	
	if (fabs(glm::length(movement)) > glm::epsilon<float>()) {
		glm::vec3 scaledMovement = glm::normalize(movement) * timeDelta * 5.0f;
		playerTransform->transform.setPosition(playerTransform->transform.getPosition() + playerTransform->transform.getRotation() * scaledMovement);
	}
	playerTransform->transform.setRotation(Util::rotateHorizontalVertical(cameraHorizontal, cameraVertical, glm::vec3(0.0f, 1.0f, 0.0f)));

	cameraSystem->update(timeDelta, entities);
	collisionUpdateSystem->update(timeDelta, entities);
	modelRenderSystem->update(timeDelta, entities);

	dynamicsWorld->stepSimulation(timeDelta);
}

void Game::handleEvent(SDL_Event& event)
{
	switch(event.type) {
	case SDL_QUIT:
		running = false;
		break;
	case SDL_MOUSEMOTION:
		cameraHorizontal -= event.motion.xrel * timeDelta * 0.2f;
		cameraVertical -= event.motion.yrel * timeDelta * 0.2f;
		cameraVertical = glm::clamp(cameraVertical, -glm::half_pi<float>() + glm::epsilon<float>(), glm::half_pi<float>() - glm::epsilon<float>());
		break;
	case SDL_TEXTINPUT:
		if (consoleIsVisible) {
			char c;
			for (int i = 0; (c = event.text.text[i]) != '\0'; i++) {
				if (c == '`') {
					// Ignore backticks
					continue;
				}
				console->inputChar(event.text.text[i]);
			}
		}
	case SDL_KEYDOWN:
		if (event.key.keysym.sym == SDLK_BACKQUOTE) {
			consoleIsVisible = !consoleIsVisible;
			if (consoleIsVisible) {
				SDL_StartTextInput();
			} else {
				SDL_StopTextInput();
			}
		}

		if (consoleIsVisible) {
			switch(event.key.keysym.sym) {
			case SDLK_RETURN:
				console->endLine();
				break;
			case SDLK_BACKSPACE:
				console->backspace();
				break;
			}
			break;
		} else if (event.key.repeat) {
			break;
		}

		switch(event.key.keysym.sym) {
		case SDLK_ESCAPE:
			SDL_SetRelativeMouseMode(SDL_FALSE);
			break;
		case SDLK_w:
			movement.z -= 1.0f;
			break;
		case SDLK_s:
			movement.z += 1.0f;
			break;
		case SDLK_d:
			movement.x += 1.0f;
			break;
		case SDLK_a:
			movement.x -= 1.0f;
			break;
		}
		break;
	case SDL_KEYUP:
		if (consoleIsVisible) {
			break;
		}

		switch(event.key.keysym.sym) {
		case SDLK_w:
			movement.z += 1.0f;
			break;
		case SDLK_s:
			movement.z -= 1.0f;
			break;
		case SDLK_d:
			movement.x -= 1.0f;
			break;
		case SDLK_a:
			movement.x += 1.0f;
			break;
		}
		break;
	case SDL_MOUSEBUTTONDOWN:
		switch(event.button.button) {
		case SDL_BUTTON_LEFT:
			SDL_SetRelativeMouseMode(SDL_TRUE);
			break;
		}
	}
}