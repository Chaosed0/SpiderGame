
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

#include "Game/Components/ModelRenderComponent.h"
#include "Game/Components/CameraComponent.h"
#include "Game/Components/CollisionComponent.h"
#include "Game/Components/TransformComponent.h"
#include "Game/Components/PlayerComponent.h"
#include "Game/Components/RigidbodyMotorComponent.h"
#include "Game/Components/FollowComponent.h"
#include "Game/Components/HealthComponent.h"
#include "Game/Components/SpiderComponent.h"
#include "Game/Components/ExpiresComponent.h"
#include "Game/Components/HurtboxComponent.h"

#include "Game/Events/HealthChangedEvent.h"

#include "Renderer/UI/Label.h"

const static int updatesPerSecond = 60;
const static int windowWidth = 1080;
const static int windowHeight = 720;

Game::Game()
{
	running = false;
	wireframe = false;
	lastUpdate = UINT32_MAX;
	accumulator = 0.0f;
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

void Game::setNoclip(bool on)
{
	if (on) {
		playerBody->setGravity(btVector3(0.0f, 0.0f, 0.0f));
	} else {
		playerBody->setGravity(dynamicsWorld->getGravity());
	}
	playerInputSystem->setNoclip(on);
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

	/*! Event Manager */
	eventManager = std::make_unique<EventManager>(world);

	/* Console */
	console = std::make_unique<Console>((float)windowWidth, windowHeight * 0.6f, (float)windowWidth, (float)windowHeight);
	console->addCallback("exit", CallbackMap::defineCallback(std::bind(&Game::exit, this)));
	console->addCallback("wireframe", CallbackMap::defineCallback<bool>(std::bind(&Game::setWireframe, this, std::placeholders::_1)));
	console->addCallback("noclip", CallbackMap::defineCallback<bool>(std::bind(&Game::setNoclip, this, std::placeholders::_1)));

	/* Renderer */
	renderer.setDebugLogCallback(std::bind(&Console::print, this->console.get(), std::placeholders::_1));
	uiRenderer.setProjection(glm::ortho(0.0f, (float)windowWidth, (float)windowHeight, 0.0f));

	/* Physics */
	btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
	btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
	btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
	dynamicsWorld->setGravity(btVector3(0.0f, -10.0f, 0.0f));

	debugDrawer.initialize();
	dynamicsWorld->setDebugDrawer(&debugDrawer);
	//debugDrawer.setDebugMode(btIDebugDraw::DBG_DrawWireframe);

	physics = std::make_unique<Physics>(dynamicsWorld, *eventManager);

	/* Scene */
	shader.compileAndLink("Shaders/basic.vert", "Shaders/lightcolor.frag");
	skinnedShader.compileAndLink("Shaders/skinned.vert", "Shaders/lightcolor.frag");
	lightShader.compileAndLink("Shaders/basic.vert", "Shaders/singlecolor.frag");
	skyboxShader.compileAndLink("Shaders/skybox.vert", "Shaders/skybox.frag");
	textShader.compileAndLink("Shaders/basic2d.vert", "Shaders/text.frag");

	glm::vec3 pointLightPositions[] = {
		glm::vec3(0.7f,  1.0f,  2.0f),
		glm::vec3(2.3f, 1.0f, -4.0f),
		glm::vec3(-4.0f,  1.0f, -12.0f),
	};

	pointLightTransforms.resize(3);
	for (unsigned i = 0; i < pointLightTransforms.size(); i++) {
		PointLight light;
		light.position = pointLightPositions[i];
		light.constant = 1.0f;
		light.linear = 0.09f;
		light.quadratic = 0.032f;
		light.ambient = glm::vec3(0.1f);
		light.diffuse = glm::vec3(0.2f);
		light.specular = glm::vec3(1.0f);
		renderer.setPointLight(i+1, light);

		pointLightTransforms[i].setPosition(pointLightPositions[i]);
		pointLightTransforms[i].setScale(glm::vec3(0.2f));
	}

	Mesh pointLightMesh = getBox(std::vector<Texture> {});
	pointLightMesh.material.setProperty("color", MaterialProperty(glm::vec4(1.0f)));
	pointLightModel = Model(std::vector<Mesh> { pointLightMesh });

	for (unsigned i = 0; i < pointLightTransforms.size(); i++) {
		unsigned lightModelHandle = renderer.getModelHandle(pointLightModel);
		unsigned lightHandle = renderer.getRenderableHandle(lightModelHandle, lightShader);
		renderer.setRenderableTransform(lightHandle, pointLightTransforms[i]);
	}

	DirLight dirLight;
	dirLight.direction = glm::vec3(0.2f, -1.0f, 0.3f);
	dirLight.ambient = glm::vec3(0.2f);
	dirLight.diffuse = glm::vec3(0.6f);
	dirLight.specular = glm::vec3(1.0f);
	renderer.setDirLight(dirLight);

	/*std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("assets/img/skybox/miramar_ft.tga");
	skyboxFaces.push_back("assets/img/skybox/miramar_bk.tga");
	skyboxFaces.push_back("assets/img/skybox/miramar_up.tga");
	skyboxFaces.push_back("assets/img/skybox/miramar_dn.tga");
	skyboxFaces.push_back("assets/img/skybox/miramar_rt.tga");
	skyboxFaces.push_back("assets/img/skybox/miramar_lf.tga");
	skyboxModel = Model(std::vector<Mesh> { getSkybox(skyboxFaces) });
	unsigned skyboxModelHandle = renderer.getModelHandle(skyboxModel);
	unsigned skyboxHandle = renderer.getRenderableHandle(skyboxModelHandle, skyboxShader);
	renderer.setRenderableTransform(skyboxHandle, Transform::identity);*/

	unsigned seed = (unsigned)time(NULL);
	this->generator.seed(seed);
	printf("USING SEED: %ud\n", seed);

	/*! Health label */
	font = std::make_shared<Font>("assets/font/Inconsolata.otf", 64);
	healthLabel = std::make_shared<Label>(font);
	healthLabel->setText("100");
	healthLabel->material.setProperty("textColor", MaterialProperty(glm::vec3(1.0f, 1.0f, 1.0f)));
	unsigned labelHandle = uiRenderer.getEntityHandle(healthLabel, textShader);
	uiRenderer.setTransform(labelHandle, Transform(glm::vec3(0.0f, windowHeight - 10.0f, 0.0f)));

	/* Test Room */
	std::uniform_int_distribution<int> seedRand(INT_MIN, INT_MAX);
	roomGenerator = RoomGenerator(seedRand(generator));
	Room room = roomGenerator.generate();
	const unsigned height = 6;

	roomData.room = room;
	roomData.meshBuilder.addRoom(room, (float)height);
	roomData.meshBuilder.construct();

	// Add the room to collision
	btCollisionShape* roomShape = roomData.meshBuilder.getCollisionMesh();
	roomData.rigidBody = new btRigidBody(0.0f, new btDefaultMotionState(), roomShape);
	dynamicsWorld->addRigidBody(roomData.rigidBody, CollisionGroupWall, CollisionGroupAll);

	// Render the room
	Texture roomTexture(TextureType_diffuse, "assets/img/brick.png");
	Model roomModel = roomData.meshBuilder.getModel(std::vector<Texture>{ roomTexture });
	roomModel.meshes[0].material.setProperty("shininess", MaterialProperty(FLT_MAX));
	unsigned roomModelHandle = renderer.getModelHandle(roomModel);
	unsigned roomRenderableHandle = renderer.getRenderableHandle(roomModelHandle, shader);

	// Make the room an entity so it registers in our Physics system
	eid_t roomEntity = world.getNewEntity();
	CollisionComponent* collisionComponent = world.addComponent<CollisionComponent>(roomEntity);
	collisionComponent->collisionObject = roomData.rigidBody;
	collisionComponent->world = dynamicsWorld;
	roomData.rigidBody->setUserPointer(new eid_t(roomEntity));

	// Initialize the player
	player = world.getNewEntity("Player");
	TransformComponent* playerTransform = world.addComponent<TransformComponent>(player);
	CollisionComponent* playerCollisionComponent = world.addComponent<CollisionComponent>(player);
	PlayerComponent* playerComponent = world.addComponent<PlayerComponent>(player);
	HealthComponent* playerHealthComponent = world.addComponent<HealthComponent>(player);
	RigidbodyMotorComponent* playerRigidbodyMotorComponent = world.addComponent<RigidbodyMotorComponent>(player);

	playerTransform->transform.setPosition(glm::vec3(0.0f, 3.0f, 0.0f));

	btCapsuleShape* shape = new btCapsuleShape(0.5f * playerTransform->transform.getScale().x, 2.0f * playerTransform->transform.getScale().y);
	btDefaultMotionState* motionState = new btDefaultMotionState(Util::gameToBt(playerTransform->transform));
	playerBody = new btRigidBody(1.0f, motionState, shape, btVector3(0.0f, 0.0f, 0.0f));
	playerBody->setAngularFactor(btVector3(0.0f, 0.0f, 0.0f));
	playerBody->setActivationState(DISABLE_DEACTIVATION);
	// This pointer is freed by the CollisionComponent destructor
	playerBody->setUserPointer(new eid_t(player));
	playerCollisionComponent->collisionObject = playerBody;
	playerCollisionComponent->world = dynamicsWorld;
	dynamicsWorld->addRigidBody(playerBody, CollisionGroupPlayer, CollisionGroupAll);

	playerRigidbodyMotorComponent->jumpSpeed = 5.0f;
	playerRigidbodyMotorComponent->moveSpeed = 5.0f;
	playerRigidbodyMotorComponent->noclip = false;

	camera = world.getNewEntity("Camera");
	TransformComponent* cameraTransformComponent = world.addComponent<TransformComponent>(camera);
	CameraComponent* cameraComponent = world.addComponent<CameraComponent>(camera);
	cameraComponent->camera = Camera(glm::radians(90.0f), windowWidth, windowHeight, 0.1f, 1000000.0f);
	cameraTransformComponent->transform.setPosition(glm::vec3(0.0f, -0.5f, 0.0f));

	playerTransform->transform.addChild(&cameraTransformComponent->transform);
	playerComponent->camera = camera;

	renderer.setCamera(&cameraComponent->camera);
	debugDrawer.setCamera(&cameraComponent->camera);

	// Load some mushrooms
	Model spiderModel = modelLoader.loadModelFromPath("assets/models/spider/spider-tex.fbx");
	std::uniform_real_distribution<float> scaleRand(0.005f, 0.010f);
	std::uniform_int_distribution<int> roomRand(0, roomData.room.boxes.size()-1);
	for (int i = 0; i < 10; i++) {
		std::stringstream namestream;
		namestream << "Spider " << i;

		eid_t spider = world.getNewEntity(namestream.str());
		ModelRenderComponent* modelComponent = world.addComponent<ModelRenderComponent>(spider);
		TransformComponent* transformComponent = world.addComponent<TransformComponent>(spider);
		CollisionComponent* collisionComponent = world.addComponent<CollisionComponent>(spider);
		FollowComponent* followComponent = world.addComponent<FollowComponent>(spider);
		RigidbodyMotorComponent* rigidbodyMotorComponent = world.addComponent<RigidbodyMotorComponent>(spider);
		HealthComponent* healthComponent = world.addComponent<HealthComponent>(spider);
		SpiderComponent* spiderComponent = world.addComponent<SpiderComponent>(spider);

		// Stick it in a random room
		RoomBox box = roomData.room.boxes[roomRand(generator)];
		std::uniform_int_distribution<int> xRand(box.left, box.right);
		std::uniform_int_distribution<int> zRand(box.bottom, box.top);
		transformComponent->transform.setPosition(glm::vec3(xRand(generator), 1.0f, zRand(generator)));
		transformComponent->transform.setScale(glm::vec3(scaleRand(generator)));

		btBoxShape* shape = new btBoxShape(btVector3(200.0f, 75.0f, 120.0f) * transformComponent->transform.getScale().x);
		btDefaultMotionState* playerMotionState = new btDefaultMotionState(Util::gameToBt(transformComponent->transform));
		btRigidBody* spiderRigidBody = new btRigidBody(1.0f, playerMotionState, shape, btVector3(0.0f, 0.0f, 0.0f));
		// This pointer is freed by the CollisionComponent destructor
		spiderRigidBody->setUserPointer(new eid_t(spider));
		dynamicsWorld->addRigidBody(spiderRigidBody, CollisionGroupEnemy, CollisionGroupAll);

		collisionComponent->collisionObject = spiderRigidBody;
		collisionComponent->world = dynamicsWorld;

		followComponent->target = playerTransform;
		rigidbodyMotorComponent->moveSpeed = 3.0f;

		unsigned int spiderModelHandle = renderer.getModelHandle(spiderModel);
		unsigned int spiderHandle = renderer.getRenderableHandle(spiderModelHandle, skinnedShader);
		renderer.setRenderableAnimation(spiderHandle, "AnimStack::idle");
		renderer.setRenderableAnimationTime(spiderHandle, i / 10.0f);
		modelComponent->renderer = &renderer;
		modelComponent->rendererHandle = spiderHandle;

		healthComponent->health = healthComponent->maxHealth = 100;
		spiderComponent->animState = SPIDER_IDLE;
		spiderComponent->attackTime = 1.0f;
	}

	shootingSystem = std::make_unique<ShootingSystem>(world, dynamicsWorld, renderer);
	playerInputSystem = std::make_unique<PlayerInputSystem>(world);
	rigidbodyMotorSystem = std::make_unique<RigidbodyMotorSystem>(world);
	modelRenderSystem = std::make_unique<ModelRenderSystem>(world, renderer);
	collisionUpdateSystem = std::make_unique<CollisionUpdateSystem>(world);
	cameraSystem = std::make_unique<CameraSystem>(world);
	followSystem = std::make_unique<FollowSystem>(world, dynamicsWorld);
	spiderSystem = std::make_unique<SpiderSystem>(world, dynamicsWorld, renderer);
	expiresSystem = std::make_unique<ExpiresSystem>(world);
	velocitySystem = std::make_unique<VelocitySystem>(world);

	spiderSystem->debugShader = lightShader;

	damageEventResponder = std::make_unique<DamageEventResponder>(world, *eventManager);
	playerJumpResponder = std::make_shared<PlayerJumpResponder>(world, *eventManager);
	hurtboxPlayerResponder = std::make_shared<HurtboxPlayerResponder>(world, *eventManager);

	std::function<void(const HealthChangedEvent& event)> healthChangedCallback =
		[world = &world, healthLabel = healthLabel](const HealthChangedEvent& event) {
			PlayerComponent* playerComponent = world->getComponent<PlayerComponent>(event.target);

			std::stringstream sstream;
			sstream << event.newHealth;
			healthLabel->setText(sstream.str());
		};
	ComponentBitmask playerComponentBitmask;
	playerComponentBitmask.setBit(world.getComponentId<PlayerComponent>(), true);
	eventManager->registerForEvent<HealthChangedEvent>(healthChangedCallback, playerComponentBitmask);

	return 0;
}

int Game::teardown()
{
	return 0;
}

int Game::loop()
{
	// Initialize lastUpdate to get an accurate time
	lastUpdate = SDL_GetTicks();
	consoleIsVisible = true;
	while (running)
	{
		// Pause while the console is visible
		if (!consoleIsVisible) {
			accumulator += SDL_GetTicks() - lastUpdate;
		}

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			this->handleEvent(event);
		}

		lastUpdate = SDL_GetTicks();
		if (accumulator >= 1000.0f / updatesPerSecond)
		{
			timeDelta = 1.0f / updatesPerSecond;
			update();
			accumulator -= 1000.0f / updatesPerSecond;
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

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	renderer.draw();
	debugDrawer.draw();

	glDisable(GL_DEPTH_TEST);
	if (consoleIsVisible) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		console->draw();
	}

	uiRenderer.draw();

	SDL_GL_SwapWindow(window);
}

void Game::update()
{
	renderer.update(timeDelta);

	playerInputSystem->update(timeDelta);
	followSystem->update(timeDelta);
	rigidbodyMotorSystem->update(timeDelta);
	velocitySystem->update(timeDelta);
	shootingSystem->update(timeDelta);

	Transform& cameraTransform = world.getComponent<TransformComponent>(camera)->transform;
	cameraTransform.setRotation(glm::angleAxis(playerInputSystem->getCameraVertical(), glm::vec3(1.0f, 0.0f, 0.0f)));

	Transform& playerTransform = world.getComponent<TransformComponent>(player)->transform;
	PointLight light;
	light.position = playerTransform.getPosition();
	light.constant = 2.0f;
	light.linear = 0.4f;
	light.quadratic = 1.0f;
	light.ambient = glm::vec3(0.2f);
	light.diffuse = glm::vec3(0.8f);
	light.specular = glm::vec3(1.0f);
	renderer.setPointLight(0, light);

	dynamicsWorld->stepSimulation(timeDelta);

	static bool feh = true;
	if (feh) {
		debugDrawer.reset();
		dynamicsWorld->debugDrawWorld();
		feh = false;
	}

	spiderSystem->update(timeDelta);

	cameraSystem->update(timeDelta);
	collisionUpdateSystem->update(timeDelta);
	modelRenderSystem->update(timeDelta);

	expiresSystem->update(timeDelta);

	world.cleanupEntities();
}

void Game::handleEvent(SDL_Event& event)
{
	switch(event.type) {
	case SDL_QUIT:
		running = false;
		break;
	case SDL_MOUSEMOTION:
		playerInputSystem->rotateCamera((float)event.motion.xrel, (float)event.motion.yrel);
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
				SDL_SetRelativeMouseMode(SDL_FALSE);
			} else {
				SDL_StopTextInput();
				SDL_SetRelativeMouseMode(SDL_TRUE);
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
			playerInputSystem->startMoving(glm::vec2(1,0));
			break;
		case SDLK_s:
			playerInputSystem->startMoving(glm::vec2(-1,0));
			break;
		case SDLK_d:
			playerInputSystem->startMoving(glm::vec2(0,1));
			break;
		case SDLK_a:
			playerInputSystem->startMoving(glm::vec2(0,-1));
			break;
		case SDLK_SPACE:
			playerInputSystem->startJump();
			break;
		}
		break;
	case SDL_KEYUP:
		if (consoleIsVisible) {
			break;
		}

		switch(event.key.keysym.sym) {
		case SDLK_w:
			playerInputSystem->stopMoving(glm::vec2(1,0));
			break;
		case SDLK_s:
			playerInputSystem->stopMoving(glm::vec2(-1,0));
			break;
		case SDLK_d:
			playerInputSystem->stopMoving(glm::vec2(0,1));
			break;
		case SDLK_a:
			playerInputSystem->stopMoving(glm::vec2(0,-1));
			break;
		}
		break;
	case SDL_MOUSEBUTTONDOWN:
		if (event.button.button == SDL_BUTTON_LEFT) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
			playerInputSystem->setShooting(true);
		}
		break;
	case SDL_MOUSEBUTTONUP:
		if (event.button.button == SDL_BUTTON_LEFT) {
			playerInputSystem->setShooting(false);
		}
		break;
	}
}

void Game::generateTestTerrain()
{
	/* Test Terrain */
	std::uniform_int_distribution<int> seedRand(INT_MIN, INT_MAX);
	const unsigned patchSize = 257;
	const float xzsize = 0.5f;
	Terrain terrain(patchSize, 0.005f, 6, 1.0f, 0.5f, seedRand(generator));
	for (unsigned i = 0; i < 4; i++) {
		this->terrainData.push_back(GameTerrainData());
		GameTerrainData& patchData = this->terrainData[i];

		glm::ivec2 origin((i % 2) -1, (i >= 2) - 1);
		glm::vec3 scale(xzsize, 20.0f, xzsize);
		glm::vec3 position(origin.x * (int)(patchSize - 1) * xzsize, 0.0f, origin.y * (int)(patchSize - 1) * xzsize);

		patchData.patch = terrain.generatePatch(origin.x, origin.y);
		patchData.model = patchData.patch.toModel(glm::ivec2(), scale);
		patchData.model.meshes[0].material.setProperty("shininess", MaterialProperty(1000000.0f));

		unsigned terrainModelHandle = renderer.getModelHandle(patchData.model);
		unsigned terrainHandle = renderer.getRenderableHandle(terrainModelHandle, shader);
		renderer.setRenderableTransform(terrainHandle, Transform(position));

		patchData.collision = patchData.patch.getCollisionData(glm::ivec2(), scale);
		patchData.vertArray = new btTriangleIndexVertexArray(patchData.collision.indices.size() / 3, patchData.collision.indices.data(), 3 * sizeof(unsigned), patchData.collision.vertices.size(), patchData.collision.vertices.data(), 3 * sizeof(float));
		patchData.shape = new btBvhTriangleMeshShape(patchData.vertArray, true);
		patchData.object = new btCollisionObject();
		patchData.object->setCollisionShape(patchData.shape);
		patchData.object->setWorldTransform(btTransform(Util::glmToBt(glm::quat()), Util::glmToBt(position)));
		dynamicsWorld->addCollisionObject(patchData.object);
	}
}
