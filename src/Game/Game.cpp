
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
#include "Game/Components/VelocityComponent.h"

#include "Game/Events/HealthChangedEvent.h"
#include "Game/Events/GemCountChangedEvent.h"
#include "Game/Events/ShotEvent.h"

#include "Renderer/UI/Label.h"
#include "Renderer/UI/UIQuad.h"

const static int updatesPerSecond = 60;
const static int windowWidth = 1080;
const static int windowHeight = 720;

static unsigned playerSourceHandle;

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

	if (!soundManager.initialize()) {
		return -1;
	}

	SDL_SetRelativeMouseMode(SDL_TRUE);

	/* Shaders */
	shader.compileAndLink("Shaders/basic.vert", "Shaders/lightcolor.frag");
	skinnedShader.compileAndLink("Shaders/skinned.vert", "Shaders/lightcolor.frag");
	lightShader.compileAndLink("Shaders/basic.vert", "Shaders/singlecolor.frag");
	skyboxShader.compileAndLink("Shaders/skybox.vert", "Shaders/skybox.frag");
	textShader.compileAndLink("Shaders/basic2d.vert", "Shaders/text.frag");
	imageShader.compileAndLink("Shaders/basic2d.vert", "Shaders/texture2d.frag");
	backShader.compileAndLink("Shaders/basic2d.vert", "Shaders/singlecolor.frag");

	/*! Event Manager */
	eventManager = std::make_unique<EventManager>(world);

	/* Console */
	std::shared_ptr<Font> font(std::make_shared<Font>("assets/font/Inconsolata.otf", 12));
	console = std::make_unique<Console>(font, glm::vec2((float)windowWidth, windowHeight * 0.6f));
	console->addCallback("exit", CallbackMap::defineCallback(std::bind(&Game::exit, this)));
	console->addCallback("wireframe", CallbackMap::defineCallback<bool>(std::bind(&Game::setWireframe, this, std::placeholders::_1)));
	console->addCallback("noclip", CallbackMap::defineCallback<bool>(std::bind(&Game::setNoclip, this, std::placeholders::_1)));
	console->addToRenderer(uiRenderer, backShader, textShader);

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
	DirLight dirLight;
	dirLight.direction = glm::vec3(0.2f, -1.0f, 0.3f);
	dirLight.ambient = glm::vec3(0.2f);
	dirLight.diffuse = glm::vec3(0.6f);
	dirLight.specular = glm::vec3(1.0f);
	renderer.setDirLight(dirLight);

	unsigned seed = (unsigned)time(NULL);
	this->generator.seed(seed);
	printf("USING SEED: %ud\n", seed);

	/* Health label */
	font = std::make_shared<Font>("assets/font/Inconsolata.otf", 50);
	healthLabel = std::make_shared<Label>(font);
	healthLabel->setText("100");
	healthLabel->material.setProperty("textColor", MaterialProperty(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)));
	healthLabel->transform.setPosition(glm::vec3(50.0f, windowHeight - 10.0f, 0.0f));
	unsigned healthLabelHandle = uiRenderer.getEntityHandle(healthLabel, textShader);

	/* Health image */
	std::shared_ptr<UIQuad> healthImage = std::make_shared<UIQuad>(Texture(TextureType_diffuse, "assets/img/heart.png"), glm::vec2(32.0f, 32.0f));
	healthImage->transform.setPosition(glm::vec3(10.0f, windowHeight - 42.0f, 0.0f));
	unsigned healthImageHandle = uiRenderer.getEntityHandle(healthImage, imageShader);

	/* Gem label */
	gemLabel = std::make_shared<Label>(font);
	gemLabel->setAlignment(Label::Alignment_right);
	gemLabel->setText("0");
	gemLabel->material.setProperty("textColor", MaterialProperty(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)));
	gemLabel->transform.setPosition(glm::vec3(windowWidth - 50.0f, windowHeight - 10.0f, 0.0f));
	unsigned gemLabelHandle = uiRenderer.getEntityHandle(gemLabel, textShader);

	/* Gem image */
	std::shared_ptr<UIQuad> gemImage = std::make_shared<UIQuad>(Texture(TextureType_diffuse, "assets/img/gem2d.png"), glm::vec2(32.0f, 32.0f));
	gemImage->transform.setPosition(glm::vec3(windowWidth - 42.0f, windowHeight - 42.0f, 0.0f));
	unsigned gemImageHandle = uiRenderer.getEntityHandle(gemImage, imageShader);

	/* Notification label */
	font = std::make_shared<Font>("assets/font/Inconsolata.otf", 30);
	std::shared_ptr<Label> facingLabel = std::make_shared<Label>(font);
	facingLabel->material.setProperty("textColor", MaterialProperty(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)));
	facingLabel->transform.setPosition(glm::vec3(windowWidth / 2.0f + 40.0f, windowHeight / 2.0f - 40.0f, 0.0f));
	unsigned facingLabelHandle = uiRenderer.getEntityHandle(facingLabel, textShader);
	font.reset();

	/* Test Room */
	std::uniform_int_distribution<int> seedRand(INT_MIN, INT_MAX);
	roomGenerator = RoomGenerator(seedRand(generator));
	Room room = roomGenerator.generate();
	const unsigned height = 6;

	roomData.room = room;
	roomData.meshBuilder.addRoom(room, (float)height);
	roomData.meshBuilder.construct();

	Model pedestalModel = modelLoader.loadModelFromPath("assets/models/pedestal.fbx");
	unsigned pedestalModelHandle = renderer.getModelHandle(pedestalModel);
	Model gemModel = modelLoader.loadModelFromPath("assets/models/gem.fbx");
	unsigned gemModelHandle = renderer.getModelHandle(gemModel);

	// Put a light in the center room and the rooms that are farthest out
	for (unsigned i = 0; i < 5; i++) {
		RoomBox box;
		if (i == 0) {
			box = room.boxes[room.leftmostBox];
		} else if (i == 1) {
			box = room.boxes[room.rightmostBox];
		} else if (i == 2) {
			box = room.boxes[room.bottommostBox];
		} else if (i == 3) {
			box = room.boxes[room.topmostBox];
		} else {
			box = room.boxes[0];
		}

		glm::vec3 center = glm::vec3(box.left + (box.right - box.left) / 2.0f, 0.0f, box.bottom + (box.top - box.bottom) / 2.0f);

		PointLight light;
		light.position = center + glm::vec3(0.0f, height / 2.0f, 0.0f);
		light.constant = 1.0f;
		light.linear = 0.18f;
		light.quadratic = 0.064f;
		light.ambient = glm::vec3(0.0f);
		light.diffuse = glm::vec3(0.4f);
		light.specular = glm::vec3(1.0f);
		renderer.setPointLight(i+1, light);

		unsigned pedestalHandle = renderer.getRenderableHandle(pedestalModelHandle, shader);
		renderer.setRenderableTransform(pedestalHandle, Transform(center));

		unsigned gemHandle = renderer.getRenderableHandle(gemModelHandle, shader);

		std::stringstream namestream;
		namestream << "Gem " << i;
		eid_t gemEntity = world.getNewEntity(namestream.str());
		TransformComponent* transformComponent = world.addComponent<TransformComponent>(gemEntity);
		CollisionComponent* collisionComponent = world.addComponent<CollisionComponent>(gemEntity);
		ModelRenderComponent* modelRenderComponent = world.addComponent<ModelRenderComponent>(gemEntity);
		VelocityComponent* velocityComponent = world.addComponent<VelocityComponent>(gemEntity);

		glm::vec3 gemPosition = center + glm::vec3(0.0f, 1.5f, 0.0f);
		btCollisionShape* gemCollisionShape = new btBoxShape(btVector3(0.1f, 0.1f, 0.05f));
		btRigidBody::btRigidBodyConstructionInfo info(0.0f, new btDefaultMotionState(btTransform(btQuaternion(), Util::glmToBt(gemPosition))), gemCollisionShape);
		btRigidBody* gemCollisionObject = new btRigidBody(info);
		gemCollisionObject->setUserPointer(new eid_t(gemEntity));
		dynamicsWorld->addRigidBody(gemCollisionObject, CollisionGroupDefault, CollisionGroupAll);

		transformComponent->transform = Transform(gemPosition);

		collisionComponent->collisionObject = gemCollisionObject;
		collisionComponent->world = dynamicsWorld;
		collisionComponent->controlsMovement = false;

		velocityComponent->angularSpeed = 1.0f;
		velocityComponent->rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);

		modelRenderComponent->rendererHandle = gemHandle;
		modelRenderComponent->renderer = &renderer;
	}

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
	eid_t roomEntity = world.getNewEntity("Level");
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

	playerTransform->transform.setPosition(glm::vec3(0.0f, 1.0f, 0.0f));

	btCapsuleShape* shape = new btCapsuleShape(0.5f * playerTransform->transform.getScale().x, 0.7f * playerTransform->transform.getScale().y);
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

	playerComponent->shotCooldown = 1.0f;
	playerComponent->shotDamage = 100;

	camera = world.getNewEntity("Camera");
	TransformComponent* cameraTransformComponent = world.addComponent<TransformComponent>(camera);
	CameraComponent* cameraComponent = world.addComponent<CameraComponent>(camera);
	cameraComponent->camera = Camera(glm::radians(90.0f), windowWidth, windowHeight, 0.1f, 1000000.0f);
	cameraTransformComponent->transform.setPosition(glm::vec3(0.0f, 0.85f, 0.0f));

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

		glm::vec3 halfExtents(200.0f, 75.0f, 120.0f);
		btCompoundShape* shape = new btCompoundShape();
		btBoxShape* boxShape = new btBoxShape(Util::glmToBt(halfExtents * transformComponent->transform.getScale().x));
		shape->addChildShape(btTransform(btQuaternion(), btVector3(0.0f, halfExtents.y * 2.0f, 0.0f) * transformComponent->transform.getScale().x), boxShape);
		btDefaultMotionState* playerMotionState = new btDefaultMotionState(Util::gameToBt(transformComponent->transform));
		btRigidBody* spiderRigidBody = new btRigidBody(10.0f, playerMotionState, shape);
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

	shootingSystem = std::make_unique<ShootingSystem>(world, dynamicsWorld, renderer, *eventManager);
	playerInputSystem = std::make_unique<PlayerInputSystem>(world, *eventManager);
	rigidbodyMotorSystem = std::make_unique<RigidbodyMotorSystem>(world);
	modelRenderSystem = std::make_unique<ModelRenderSystem>(world, renderer);
	collisionUpdateSystem = std::make_unique<CollisionUpdateSystem>(world);
	cameraSystem = std::make_unique<CameraSystem>(world);
	followSystem = std::make_unique<FollowSystem>(world, dynamicsWorld);
	spiderSystem = std::make_unique<SpiderSystem>(world, dynamicsWorld, renderer);
	expiresSystem = std::make_unique<ExpiresSystem>(world);
	velocitySystem = std::make_unique<VelocitySystem>(world);
	playerFacingSystem = std::make_unique<PlayerFacingSystem>(world, dynamicsWorld, facingLabel);

	spiderSystem->debugShader = lightShader;

	damageEventResponder = std::make_unique<DamageEventResponder>(world, *eventManager);
	playerJumpResponder = std::make_shared<PlayerJumpResponder>(world, *eventManager);
	hurtboxPlayerResponder = std::make_shared<HurtboxPlayerResponder>(world, *eventManager);

	AudioClip shotClip("assets/sound/hvylas.wav");
	playerSourceHandle = soundManager.getSourceHandle();
	std::function<void(const ShotEvent& event)> shotCallback =
		[world = &world, soundManager = &soundManager, shotClip](const ShotEvent& event) {
			TransformComponent* transformComponent = world->getComponent<TransformComponent>(event.source);
			soundManager->playClipAtSource(shotClip, playerSourceHandle);
		};
	eventManager->registerForEvent<ShotEvent>(shotCallback);

	std::function<void(const HealthChangedEvent& event)> healthChangedCallback =
		[world = &world, healthLabel = healthLabel](const HealthChangedEvent& event) {
			PlayerComponent* playerComponent = world->getComponent<PlayerComponent>(event.entity);

			std::stringstream sstream;
			sstream << event.newHealth;
			healthLabel->setText(sstream.str());
		};
	eventManager->registerForEvent<HealthChangedEvent>(healthChangedCallback);

	std::function<void(const GemCountChangedEvent& event)> gemCountChangedCallback =
		[world = &world, gemLabel = gemLabel](const GemCountChangedEvent& event) {
			std::stringstream sstream;
			sstream << event.newGemCount;
			gemLabel->setText(sstream.str());
		};
	eventManager->registerForEvent<GemCountChangedEvent>(gemCountChangedCallback);

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
	console->setVisible(true);
	while (running)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			this->handleEvent(event);
		}

		// Pause while the console is visible
		if (!console->isVisible()) {
			accumulator += SDL_GetTicks() - lastUpdate;

			lastUpdate = SDL_GetTicks();
			if (accumulator >= 1000.0f / updatesPerSecond)
			{
				timeDelta = 1.0f / updatesPerSecond;
				update();
				accumulator -= 1000.0f / updatesPerSecond;
			}
		}
	
		draw();
	}

	return 0;
}

void Game::draw()
{
	glPolygonMode(GL_FRONT_AND_BACK, this->wireframe ? GL_LINE : GL_FILL);

	renderer.draw();
	debugDrawer.draw();
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
	soundManager.setSourcePosition(playerSourceHandle, playerTransform.getPosition());
	soundManager.setListenerTransform(playerTransform);
	soundManager.update();

	PointLight light;
	light.position = playerTransform.getPosition();
	light.constant = 2.0f;
	light.linear = 0.2f;
	light.quadratic = 0.5f;
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
	playerFacingSystem->update(timeDelta);
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
		if (console->isVisible()) {
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
			console->setVisible(!console->isVisible());
			if (console->isVisible()) {
				SDL_StartTextInput();
				SDL_SetRelativeMouseMode(SDL_FALSE);
			} else {
				SDL_StopTextInput();
				SDL_SetRelativeMouseMode(SDL_TRUE);
			}
		}

		if (console->isVisible()) {
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
		case SDLK_e:
			playerInputSystem->activate();
			break;
		case SDLK_SPACE:
			playerInputSystem->startJump();
			break;
		}
		break;
	case SDL_KEYUP:
		if (console->isVisible()) {
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
