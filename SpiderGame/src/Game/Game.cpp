
#include "Game.h"

#include <SDL.h>
#include <SDL_Image.h>

#include <cstdio>
#include <cstdint>
#include <sstream>
#include <random>
#include <ctime>

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
#include "Game/Components/AudioListenerComponent.h"
#include "Game/Components/AudioSourceComponent.h"
#include "Game/Components/PointLightComponent.h"

#include "Game/Events/HealthChangedEvent.h"
#include "Game/Events/GemCountChangedEvent.h"
#include "Game/Events/BulletCountChangedEvent.h"
#include "Game/Events/ShotEvent.h"

#include "Renderer/UI/Label.h"
#include "Renderer/UI/UIQuad.h"

const static int updatesPerSecond = 60;
const static int windowWidth = 1080;
const static int windowHeight = 720;

const static int gemCount = 3;
const static int spiderCount = 6;

glm::vec3 roomBoxCenter(const RoomBox& box);

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
	eid_t player = world.getEntityWithName("Player");
	if (player == World::NullEntity) {
		return;
	}

	CollisionComponent* collisionComponent = world.getComponent<CollisionComponent>(player);
	if (collisionComponent == nullptr) {
		return;
	}

	btRigidBody* rigidBody = (btRigidBody*)collisionComponent->collisionObject;
	if (on) {
		rigidBody->setGravity(btVector3(0.0f, 0.0f, 0.0f));
	} else {
		rigidBody->setGravity(dynamicsWorld->getGravity());
	}
	playerInputSystem->setNoclip(on);
}

void Game::setBulletDebugDraw(bool on)
{
	if (on) {
		debugDrawer.setDebugMode(btIDebugDraw::DBG_DrawWireframe);
	} else {
		debugDrawer.setDebugMode(0);
	}
}

void Game::refreshBulletDebugDraw()
{
	debugDrawer.reset();
	dynamicsWorld->debugDrawWorld();
}

int Game::setup()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC | SDL_INIT_JOYSTICK) < 0)
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

	/* Input */
	input.initialize();
	input.setDefaultMapping("Horizontal", KbmAxis_D, KbmAxis_A);
	input.setDefaultMapping("Vertical", KbmAxis_W, KbmAxis_S);
	input.setDefaultMapping("LookHorizontal", KbmAxis_MouseXPos, KbmAxis_MouseXNeg, AxisProps(0.1f, 0.2f, 0.3f));
	input.setDefaultMapping("LookVertical", KbmAxis_MouseYPos, KbmAxis_MouseYNeg, AxisProps(0.1f, 0.2f, 0.3f));
	input.setDefaultMapping("Jump", KbmAxis_Space, KbmAxis_None);
	input.setDefaultMapping("Use", KbmAxis_E, KbmAxis_None);
	input.setDefaultMapping("Fire", KbmAxis_MouseLeft, KbmAxis_None);

	input.setDefaultMapping("Horizontal", ControllerAxis_LStickXPos, ControllerAxis_LStickXNeg);
	input.setDefaultMapping("Vertical", ControllerAxis_LStickYPos, ControllerAxis_LStickYNeg);
	input.setDefaultMapping("LookHorizontal", ControllerAxis_RStickXPos, ControllerAxis_RStickXNeg, AxisProps(3.0f, 0.2f, 0.3f));
	input.setDefaultMapping("LookVertical", ControllerAxis_RStickYPos, ControllerAxis_RStickYNeg, AxisProps(3.0f, 0.2f, 0.3f));
	input.setDefaultMapping("Jump", ControllerAxis_A, ControllerAxis_None);
	input.setDefaultMapping("Use", ControllerAxis_X, ControllerAxis_None);
	input.setDefaultMapping("Fire", ControllerAxis_RightTrigger, ControllerAxis_None);

	/* Shaders */
	ShaderLoader shaderLoader;
	shader = shaderLoader.compileAndLink("Shaders/basic.vert", "Shaders/lightcolor.frag");
	skinnedShader = shaderLoader.compileAndLink("Shaders/skinned.vert", "Shaders/lightcolor.frag");
	lightShader = shaderLoader.compileAndLink("Shaders/basic.vert", "Shaders/singlecolor.frag");
	skyboxShader = shaderLoader.compileAndLink("Shaders/skybox.vert", "Shaders/skybox.frag");
	textShader = shaderLoader.compileAndLink("Shaders/basic2d.vert", "Shaders/text.frag");
	imageShader = shaderLoader.compileAndLink("Shaders/basic2d.vert", "Shaders/texture2d.frag");
	backShader = shaderLoader.compileAndLink("Shaders/basic2d.vert", "Shaders/singlecolor.frag");

	TextureLoader textureLoader;

	Material defaultMaterial;
	defaultMaterial.setProperty("shininess", FLT_MAX);
	defaultMaterial.setProperty("diffuseTint", glm::vec3(1.0f));
	defaultMaterial.setProperty("texture_specular", textureLoader.loadFromFile(TextureType_specular, "assets/img/default_specular.png"));
	modelLoader.setDefaultMaterialProperties(defaultMaterial);

	/*! Event Manager */
	eventManager = std::make_unique<EventManager>(world);

	/* Console */
	std::shared_ptr<Font> font(std::make_shared<Font>("assets/font/Inconsolata.otf", 12));
	console = std::make_unique<Console>(font, glm::vec2((float)windowWidth, windowHeight * 0.6f));
	console->addCallback("exit", CallbackMap::defineCallback(std::bind(&Game::exit, this)));
	console->addCallback("wireframe", CallbackMap::defineCallback<bool>(std::bind(&Game::setWireframe, this, std::placeholders::_1)));
	console->addCallback("noclip", CallbackMap::defineCallback<bool>(std::bind(&Game::setNoclip, this, std::placeholders::_1)));
	console->addCallback("enableBulletDebugDraw", CallbackMap::defineCallback<bool>(std::bind(&Game::setBulletDebugDraw, this, std::placeholders::_1)));
	console->addCallback("refreshBulletDebugDraw", CallbackMap::defineCallback(std::bind(&Game::refreshBulletDebugDraw, this)));
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
	gui.healthLabel = std::make_shared<Label>(font);
	gui.healthLabel->setText("100");
	gui.healthLabel->material.setProperty("textColor", MaterialProperty(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)));
	gui.healthLabel->transform = Transform(glm::vec3(50.0f, windowHeight - 10.0f, 0.0f)).matrix();
	gui.healthLabelHandle = uiRenderer.getEntityHandle(gui.healthLabel, textShader);

	/* Health image */
	gui.healthImage = std::make_shared<UIQuad>(textureLoader.loadFromFile(TextureType_diffuse, "assets/img/heart.png"), glm::vec2(32.0f, 32.0f));
	gui.healthImage->transform = Transform(glm::vec3(10.0f, windowHeight - 42.0f, 0.0f)).matrix();
	gui.healthImageHandle = uiRenderer.getEntityHandle(gui.healthImage, imageShader);

	/* Gem label */
	gui.gemLabel = std::make_shared<Label>(font);
	gui.gemLabel->setAlignment(Label::Alignment_right);
	gui.gemLabel->setText("0");
	gui.gemLabel->material.setProperty("textColor", MaterialProperty(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)));
	gui.gemLabel->transform = Transform(glm::vec3(windowWidth - 50.0f, windowHeight - 10.0f, 0.0f)).matrix();
	gui.gemLabelHandle = uiRenderer.getEntityHandle(gui.gemLabel, textShader);

	/* Gem image */
	gui.gemImage = std::make_shared<UIQuad>(textureLoader.loadFromFile(TextureType_diffuse, "assets/img/gem2d.png"), glm::vec2(32.0f, 32.0f));
	gui.gemImage->transform = Transform(glm::vec3(windowWidth - 42.0f, windowHeight - 42.0f, 0.0f)).matrix();
	gui.gemImageHandle = uiRenderer.getEntityHandle(gui.gemImage, imageShader);

	/* Bullet label */
	gui.bulletLabel = std::make_shared<Label>(font);
	gui.bulletLabel->setText("0");
	gui.bulletLabel->material.setProperty("textColor", MaterialProperty(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)));
	gui.bulletLabel->transform = Transform(glm::vec3(windowWidth / 2.0f, windowHeight - 10.0f, 0.0f)).matrix();
	gui.bulletLabelHandle = uiRenderer.getEntityHandle(gui.bulletLabel, textShader);

	/* Bullet image */
	gui.bulletImage = std::make_shared<UIQuad>(textureLoader.loadFromFile(TextureType_diffuse, "assets/img/bullet.png"), glm::vec2(32.0f, 32.0f));
	gui.bulletImage->transform = Transform(glm::vec3(windowWidth / 2.0f - 40.0f, windowHeight - 42.0f, 0.0f)).matrix();
	gui.bulletImageHandle = uiRenderer.getEntityHandle(gui.bulletImage, imageShader);

	/* Notification label */
	font = std::make_shared<Font>("assets/font/Inconsolata.otf", 30);
	gui.facingLabel = std::make_shared<Label>(font);
	gui.facingLabel->material.setProperty("textColor", MaterialProperty(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)));
	gui.facingLabel->transform = Transform(glm::vec3(windowWidth / 2.0f + 40.0f, windowHeight / 2.0f - 40.0f, 0.0f)).matrix();
	gui.facingLabelHandle = uiRenderer.getEntityHandle(gui.facingLabel, textShader);
	font.reset();

	/* Aiming reticle*/
	gui.reticleImage = std::make_shared<UIQuad>(textureLoader.loadFromFile(TextureType_diffuse, "assets/img/reticle.png"), glm::vec2(32.0f, 32.0f));
	gui.reticleImage->transform = Transform(glm::vec3(windowWidth / 2.0f, windowHeight / 2.0f, 0.0f)).matrix();
	gui.reticleHandle = uiRenderer.getEntityHandle(gui.reticleImage, imageShader);

	/* Test Room */
	std::uniform_int_distribution<int> seedRand(INT_MIN, INT_MAX);
	roomGenerator = RoomGenerator(seedRand(generator));
	Room room = roomGenerator.generate();
	const unsigned roomHeight = 6;

	roomData.room = room;
	roomData.meshBuilder.addRoom(room, (float)roomHeight);
	roomData.meshBuilder.construct();

	Model pedestalModel = modelLoader.loadModelFromPath("assets/models/pedestal.fbx");
	Renderer::ModelHandle pedestalModelHandle = renderer.getModelHandle(pedestalModel);
	Model gemModel = modelLoader.loadModelFromPath("assets/models/gem.fbx");
	gemModel.material.setProperty("shininess", 32.0f);

	const RoomBox& centerRoomBox = room.boxes[0];
	const RoomBox& topmostRoomBox = room.boxes[room.topmostBox];
	const RoomBox& bottommostRoomBox = room.boxes[room.bottommostBox];
	const RoomBox& rightmostRoomBox = room.boxes[room.rightmostBox];
	const RoomBox& leftmostRoomBox = room.boxes[room.leftmostBox];

	std::vector<glm::vec3> gemFloorPositions = {
		roomBoxCenter(leftmostRoomBox),
		roomBoxCenter(rightmostRoomBox),
		roomBoxCenter(bottommostRoomBox)
	};

	std::vector<glm::vec3> gemColors = {
		glm::vec3(1.0f, 0.0f, 0.2f),
		glm::vec3(0.2f, 1.0f, 0.0f),
		glm::vec3(0.0f, 0.2f, 1.0f)
	};

	// Put a light in the center room and the rooms that are farthest out
	for (unsigned i = 0; i < gemFloorPositions.size(); i++) {
		glm::vec3 floorPosition = gemFloorPositions[i];
		glm::vec3 color = gemColors[i];

		PointLight light;
		light.position = floorPosition + glm::vec3(0.0f, roomHeight / 2.0f, 0.0f);
		light.constant = 1.0f;
		light.linear = 0.18f;
		light.quadratic = 0.064f;
		light.ambient = color * 0.1f;
		light.diffuse = color * 0.6f;
		light.specular = color * 1.0f;
		renderer.setPointLight(i+1, light);

		Renderer::RenderableHandle pedestalHandle = renderer.getRenderableHandle(pedestalModelHandle, shader);
		eid_t pedestalEntity = world.getNewEntity();
		TransformComponent* pedestalTransformComponent = world.addComponent<TransformComponent>(pedestalEntity);
		ModelRenderComponent* pedestalModelComponent = world.addComponent<ModelRenderComponent>(pedestalEntity);
		pedestalTransformComponent->transform->setPosition(floorPosition);
		pedestalModelComponent->rendererHandle = pedestalHandle;

		gemModel.material.setProperty("diffuseTint", color);
		Renderer::ModelHandle gemModelHandle = renderer.getModelHandle(gemModel);
		Renderer::RenderableHandle gemHandle = renderer.getRenderableHandle(gemModelHandle, shader);

		std::stringstream namestream;
		namestream << "Gem " << i;
		eid_t gemEntity = world.getNewEntity(namestream.str());
		TransformComponent* transformComponent = world.addComponent<TransformComponent>(gemEntity);
		CollisionComponent* collisionComponent = world.addComponent<CollisionComponent>(gemEntity);
		ModelRenderComponent* modelRenderComponent = world.addComponent<ModelRenderComponent>(gemEntity);
		VelocityComponent* velocityComponent = world.addComponent<VelocityComponent>(gemEntity);

		glm::vec3 gemPosition = floorPosition + glm::vec3(0.0f, 1.5f, 0.0f);
		btCollisionShape* gemCollisionShape = new btBoxShape(btVector3(0.1f, 0.1f, 0.05f));
		btRigidBody::btRigidBodyConstructionInfo info(0.0f, new btDefaultMotionState(btTransform(btQuaternion::getIdentity(), Util::glmToBt(gemPosition))), gemCollisionShape);
		btRigidBody* gemCollisionObject = new btRigidBody(info);
		gemCollisionObject->setUserPointer(new eid_t(gemEntity));
		dynamicsWorld->addRigidBody(gemCollisionObject, CollisionGroupDefault, CollisionGroupAll);

		transformComponent->transform->setPosition(gemPosition);

		collisionComponent->collisionObject = gemCollisionObject;
		collisionComponent->world = dynamicsWorld;
		collisionComponent->controlsMovement = false;

		velocityComponent->angularSpeed = 1.0f;
		velocityComponent->rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);

		modelRenderComponent->rendererHandle = gemHandle;
	}
	PointLight light;
	light.constant = 1.0f;
	light.linear = 0.1f;
	light.quadratic = 0.25f;
	light.ambient = glm::vec3(0.2f);
	light.diffuse = glm::vec3(0.8f);
	light.specular = glm::vec3(1.0f);
	renderer.setPointLight(0, light);

	// Clutter
	Model barrelModel = modelLoader.loadModelFromPath("assets/models/barrel.fbx");
	barrelModel.material.setProperty("shininess", 16.0f);
	Renderer::ModelHandle barrelModelHandle = renderer.getModelHandle(barrelModel);
	std::uniform_real_distribution<float> barrelRand(0.0f, 1.0f);
	for (unsigned i = 0; i < room.sides.size(); i++) {
		RoomSide& side = room.sides[i];

		// Don't clutter the extreme rooms, we want to put other things there
		std::vector<RoomBox> extremeRooms = { leftmostRoomBox, rightmostRoomBox, topmostRoomBox, bottommostRoomBox };
		bool skip = false;
		for (unsigned i = 0; i < extremeRooms.size(); i++) {
			if (side.x1 <= extremeRooms[i].right && side.x0 >= extremeRooms[i].left &&
				side.y1 <= extremeRooms[i].top && side.y0 >= extremeRooms[i].bottom)
			{
				skip = true;
				break;
			}
		}

		if (skip) {
			continue;
		}

		bool horizontal = (side.y0 == side.y1);
		int min = horizontal ? side.x0 : side.y0;
		int max = horizontal ? side.x1 : side.y1;
		for (int i = min; i < max; ++i) {
			float rand = barrelRand(generator);
			int x = horizontal ? i : side.x0 + side.normal.x;
			int y = horizontal ? side.y0 + side.normal.y : i;
			if (rand < 0.1f) {
				eid_t entity = world.getNewEntity("Barrel");
				TransformComponent* transformComponent = world.addComponent<TransformComponent>(entity);
				CollisionComponent* collisionComponent = world.addComponent<CollisionComponent>(entity);
				ModelRenderComponent* modelRenderComponent = world.addComponent<ModelRenderComponent>(entity);

				Renderer::RenderableHandle barrelHandle = renderer.getRenderableHandle(barrelModelHandle, shader);
				modelRenderComponent->rendererHandle = barrelHandle;

				btVector3 halfExtents(0.5f, 0.75f, 0.5f);
				btCollisionShape* collisionShape = new btBoxShape(halfExtents);
				btCompoundShape* compoundShape = new btCompoundShape();
				compoundShape->addChildShape(btTransform(btQuaternion::getIdentity(), btVector3(0.0f, halfExtents.y(), 0.0f)), collisionShape);
				btRigidBody::btRigidBodyConstructionInfo info(0.0f, new btDefaultMotionState(btTransform(btQuaternion::getIdentity(), btVector3((float)x, 0, (float)y))), compoundShape);
				btRigidBody* collisionObject = new btRigidBody(info);
				collisionObject->setUserPointer(new eid_t(entity));
				dynamicsWorld->addRigidBody(collisionObject, CollisionGroupDefault, CollisionGroupAll);

				collisionComponent->collisionObject = collisionObject;
				collisionComponent->world = dynamicsWorld;
			}
		}
	}

	// Add the room to collision
	btCollisionShape* roomShape = roomData.meshBuilder.getCollisionMesh();
	roomData.rigidBody = new btRigidBody(0.0f, new btDefaultMotionState(), roomShape);
	dynamicsWorld->addRigidBody(roomData.rigidBody, CollisionGroupWall, CollisionGroupAll);

	// Render the room
	Texture roomTexture(textureLoader.loadFromFile(TextureType_diffuse, "assets/img/brick.png"));
	Model roomModel = roomData.meshBuilder.getModel(std::vector<Texture>{ roomTexture });
	roomModel.material.setProperty("shininess", MaterialProperty(FLT_MAX));
	Renderer::ModelHandle roomModelHandle = renderer.getModelHandle(roomModel);
	Renderer::RenderableHandle roomRenderableHandle = renderer.getRenderableHandle(roomModelHandle, shader);

	// Make the room an entity so it registers in our Physics system
	eid_t roomEntity = world.getNewEntity("Level");
	CollisionComponent* collisionComponent = world.addComponent<CollisionComponent>(roomEntity);
	ModelRenderComponent* modelComponent = world.addComponent<ModelRenderComponent>(roomEntity);

	collisionComponent->collisionObject = roomData.rigidBody;
	collisionComponent->world = dynamicsWorld;
	roomData.rigidBody->setUserPointer(new eid_t(roomEntity));

	modelComponent->rendererHandle = roomRenderableHandle;

	// Put down a table with bullets
	glm::vec3 tableDimensions(2.0f, 1.0f, 0.785f);
	Model tableModel = modelLoader.loadModelFromPath("assets/models/table.fbx");
	Renderer::ModelHandle tableModelHandle = renderer.getModelHandle(tableModel);

	glm::vec3 bulletDimensions(0.2f, 0.07f, 0.2f);
	Model bulletModel = modelLoader.loadModelFromPath("assets/models/bullets.fbx");
	Renderer::ModelHandle bulletModelHandle = renderer.getModelHandle(bulletModel);

	std::vector<Transform> bulletSpawnLocations = {
		Transform(roomBoxCenter(topmostRoomBox)),
		Transform(glm::vec3(leftmostRoomBox.left + 2.0f, 0.0f, (leftmostRoomBox.bottom + leftmostRoomBox.top) / 2.0f), glm::angleAxis(glm::half_pi<float>(), Util::up)),
		Transform(glm::vec3((bottommostRoomBox.left + bottommostRoomBox.right) / 2.0f, 0.0f, bottommostRoomBox.bottom + 2.0f)),
		Transform(glm::vec3(rightmostRoomBox.right - 2.0f, 0.0f, (rightmostRoomBox.bottom + rightmostRoomBox.top) / 2.0f), glm::angleAxis(glm::half_pi<float>(), Util::up)),
	};

	for (unsigned i = 0; i < bulletSpawnLocations.size(); i++) {
		Transform initialTransform = bulletSpawnLocations[i];

		// Table
		eid_t table = world.getNewEntity("Table");
		TransformComponent* tableTransformComponent = world.addComponent<TransformComponent>(table);
		CollisionComponent* tableCollisionComponent = world.addComponent<CollisionComponent>(table);
		ModelRenderComponent* tableModelComponent = world.addComponent<ModelRenderComponent>(table);

		btCollisionShape* tableCollisionShape = new btBoxShape(Util::glmToBt(tableDimensions / 2.0f));
		btCompoundShape* tableCompoundShape = new btCompoundShape();
		tableCompoundShape->addChildShape(btTransform(btQuaternion::getIdentity(), btVector3(0.0f, tableDimensions.y / 2.0f, 0.0f)), tableCollisionShape);

		btRigidBody::btRigidBodyConstructionInfo tableInfo(0.0f, new btDefaultMotionState(Util::gameToBt(initialTransform)), tableCompoundShape);
		btRigidBody* tableCollisionObject = new btRigidBody(tableInfo);
		tableCollisionObject->setUserPointer(new eid_t(table));
		dynamicsWorld->addRigidBody(tableCollisionObject, CollisionGroupDefault, CollisionGroupAll);

		tableCollisionComponent->collisionObject = tableCollisionObject;
		tableCollisionComponent->world = dynamicsWorld;

		Renderer::RenderableHandle tableHandle = renderer.getRenderableHandle(tableModelHandle, shader);
		tableModelComponent->rendererHandle = tableHandle;

		// Bullet
		glm::vec3 bulletPosition(initialTransform.getPosition() + glm::vec3(0.0f, tableDimensions.y + bulletDimensions.y / 2.0f, 0.0f));
		eid_t bullet = world.getNewEntity("Bullets");
		TransformComponent* bulletTransformComponent = world.addComponent<TransformComponent>(bullet);
		CollisionComponent* bulletCollisionComponent = world.addComponent<CollisionComponent>(bullet);
		ModelRenderComponent* bulletModelComponent = world.addComponent<ModelRenderComponent>(bullet);

		std::uniform_real_distribution<float> angleRand(-glm::half_pi<float>(), glm::half_pi<float>());
		btCollisionShape* bulletCollisionShape = new btBoxShape(Util::glmToBt(bulletDimensions / 2.0f));
		btCompoundShape* bulletCompoundShape = new btCompoundShape();
		bulletCompoundShape->addChildShape(btTransform(btQuaternion::getIdentity(), btVector3(0.0f, bulletDimensions.y / 2.0f, 0.0f)), bulletCollisionShape);

		btTransform bulletTransform(btQuaternion(btVector3(0.0f, 1.0f, 0.0f), angleRand(generator)), Util::glmToBt(bulletPosition));
		btRigidBody::btRigidBodyConstructionInfo bulletInfo(0.0f, new btDefaultMotionState(bulletTransform), bulletCompoundShape);
		btRigidBody* bulletCollisionObject = new btRigidBody(bulletInfo);
		bulletCollisionObject->setUserPointer(new eid_t(bullet));
		dynamicsWorld->addRigidBody(bulletCollisionObject, CollisionGroupDefault, CollisionGroupAll);

		bulletCollisionComponent->collisionObject = bulletCollisionObject;
		bulletCollisionComponent->world = dynamicsWorld;

		Renderer::RenderableHandle bulletHandle = renderer.getRenderableHandle(bulletModelHandle, shader);
		bulletModelComponent->rendererHandle = bulletHandle;
	}

	// Initialize the player
	glm::vec3 playerSpawn = glm::vec3((topmostRoomBox.left + topmostRoomBox.right) / 2.0f, 0.5f, topmostRoomBox.top - 1.0f);

	eid_t player = world.getNewEntity("Player");
	TransformComponent* playerTransform = world.addComponent<TransformComponent>(player);
	CollisionComponent* playerCollisionComponent = world.addComponent<CollisionComponent>(player);
	PlayerComponent* playerComponent = world.addComponent<PlayerComponent>(player);
	HealthComponent* playerHealthComponent = world.addComponent<HealthComponent>(player);
	RigidbodyMotorComponent* playerRigidbodyMotorComponent = world.addComponent<RigidbodyMotorComponent>(player);
	AudioListenerComponent* playerAudioListenerComponent = world.addComponent<AudioListenerComponent>(player);
	AudioSourceComponent* playerAudioSourceComponent = world.addComponent<AudioSourceComponent>(player);

	playerTransform->transform->setPosition(playerSpawn);

	btCapsuleShape* shape = new btCapsuleShape(0.5f * playerTransform->transform->getScale().x, 0.7f * playerTransform->transform->getScale().y);
	btDefaultMotionState* motionState = new btDefaultMotionState(Util::gameToBt(*playerTransform->transform));
	btRigidBody* playerBody = new btRigidBody(5.0f, motionState, shape, btVector3(0.0f, 0.0f, 0.0f));
	playerBody->setContactProcessingThreshold(0.0f);
	playerBody->setAngularFactor(btVector3(0.0f, 0.0f, 0.0f));
	playerBody->setActivationState(DISABLE_DEACTIVATION);
	// This pointer is freed by the CollisionComponent destructor
	playerBody->setUserPointer(new eid_t(player));
	playerCollisionComponent->collisionObject = playerBody;
	playerCollisionComponent->world = dynamicsWorld;
	dynamicsWorld->addRigidBody(playerBody, CollisionGroupPlayer, CollisionGroupAll);

	playerRigidbodyMotorComponent->jumpSpeed = 5.0f;
	playerRigidbodyMotorComponent->moveSpeed = 5.0f;

	playerComponent->shotCooldown = 0.3f;
	playerComponent->shotDamage = 100;
	playerComponent->shotClip = AudioClip("assets/sound/shot.wav");
	playerComponent->dryFireClip = AudioClip("assets/sound/dryfire.wav");
	playerComponent->hurtClip = AudioClip("assets/sound/minecraft/classic_hurt.ogg");
	playerComponent->gemPickupClip = AudioClip("assets/sound/pickup.wav");

	eid_t camera = world.getNewEntity("Camera");
	TransformComponent* cameraTransformComponent = world.addComponent<TransformComponent>(camera);
	CameraComponent* cameraComponent = world.addComponent<CameraComponent>(camera);
	cameraComponent->camera = Camera(glm::radians(90.0f), windowWidth, windowHeight, 0.1f, 1000000.0f);
	cameraTransformComponent->transform->setPosition(glm::vec3(0.0f, 0.85f, 0.0f));

	cameraTransformComponent->transform->setParent(playerTransform->transform);
	playerComponent->camera = camera;

	SoundManager::SourceHandle playerSourceHandle = soundManager.getSourceHandle();
	playerAudioSourceComponent->sourceHandle = playerSourceHandle;

	renderer.setCamera(&cameraComponent->camera);
	debugDrawer.setCamera(&cameraComponent->camera);

	eid_t playerLight = world.getNewEntity("PlayerLight");
	TransformComponent* lightTransform = world.addComponent<TransformComponent>(playerLight);
	PointLightComponent* lightComponent = world.addComponent<PointLightComponent>(playerLight);
	lightTransform->transform->setParent(playerTransform->transform);
	lightComponent->pointLightIndex = 0;

	eid_t playerGun = world.getNewEntity("PlayerGun");
	TransformComponent* gunTransform = world.addComponent<TransformComponent>(playerGun);
	ModelRenderComponent* gunModelComponent = world.addComponent<ModelRenderComponent>(playerGun);

	gunTransform->transform->setPosition(glm::vec3(0.2f, -0.3f, -0.15f));
	gunTransform->transform->setParent(cameraTransformComponent->transform);

	Model gunModel = modelLoader.loadModelFromPath("assets/models/gun.fbx");
	auto gunModelHandle = renderer.getModelHandle(gunModel);
	auto gunRenderableHandle = renderer.getRenderableHandle(gunModelHandle, shader);
	gunModelComponent->rendererHandle = gunRenderableHandle;

	// This was gotten from blender - isn't my game engine awesome
	glm::vec3 gunBarrelEnd(0.0f, 0.19f, -0.665f);
	playerComponent->gun = playerGun;
	playerComponent->gunBarrelOffset = gunBarrelEnd;

	// Create gunFX
	Vertex fromVert;
	fromVert.position = glm::vec3(0.0f);
	Vertex toVert;
	toVert.position = Util::forward * 3.0f;
	Mesh lineMesh(std::vector<Vertex>{fromVert, toVert}, std::vector<unsigned>{0,1});
	Material material;
	material.drawType = MaterialDrawType_Lines;
	material.setProperty("color", MaterialProperty(glm::vec4(0.5f, 0.5f, 0.0f, 1.0f)));
	auto bulletMeshHandle = renderer.getModelHandle(Model(lineMesh, material));
	playerComponent->shotTracerModelHandle = bulletMeshHandle;
	playerComponent->tracerShader = lightShader;

	Texture muzzleFlashTexture(textureLoader.loadFromFile(TextureType_diffuse, "assets/img/flash.png"));
	Model muzzleFlashPlane = getPlane(std::vector<Texture> { muzzleFlashTexture }, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.3f, 0.3f));
	auto muzzleFlashModelHandle = renderer.getModelHandle(muzzleFlashPlane);
	playerComponent->muzzleFlashModelHandle = muzzleFlashModelHandle;
	playerComponent->flashShader = shader;

	shootingSystem = std::make_unique<ShootingSystem>(world, dynamicsWorld, renderer, *eventManager, generator);
	playerInputSystem = std::make_unique<PlayerInputSystem>(world, input, *eventManager);
	rigidbodyMotorSystem = std::make_unique<RigidbodyMotorSystem>(world);
	modelRenderSystem = std::make_unique<ModelRenderSystem>(world, renderer);
	collisionUpdateSystem = std::make_unique<CollisionUpdateSystem>(world);
	cameraSystem = std::make_unique<CameraSystem>(world);
	followSystem = std::make_unique<FollowSystem>(world, dynamicsWorld, roomData.room);
	spiderSystem = std::make_unique<SpiderSystem>(world, *eventManager, dynamicsWorld, renderer, soundManager, generator);
	expiresSystem = std::make_unique<ExpiresSystem>(world);
	velocitySystem = std::make_unique<VelocitySystem>(world);
	playerFacingSystem = std::make_unique<PlayerFacingSystem>(world, dynamicsWorld, gui.facingLabel);
	audioListenerSystem = std::make_unique<AudioListenerSystem>(world, soundManager);
	audioSourceSystem = std::make_unique<AudioSourceSystem>(world, soundManager);
	pointLightSystem = std::make_unique<PointLightSystem>(world, renderer);

	spiderSystem->debugShader = lightShader;

	spiderSpawner = std::make_unique<SpiderSpawner>(renderer, soundManager, world, dynamicsWorld, modelLoader, generator, skinnedShader, roomData.room, player);

	damageEventResponder = std::make_unique<DamageEventResponder>(world, *eventManager);
	playerJumpResponder = std::make_shared<PlayerJumpResponder>(world, *eventManager);
	hurtboxPlayerResponder = std::make_shared<HurtboxPlayerResponder>(world, *eventManager);

	std::function<void(const ShotEvent& event)> shotCallback =
		[world = &world, soundManager = &soundManager](const ShotEvent& event) {
			PlayerComponent* playerComponent = world->getComponent<PlayerComponent>(event.source);
			AudioSourceComponent* audioSourceComponent = world->getComponent<AudioSourceComponent>(event.source);
			AudioClip clip;
			if (event.actuallyShot) {
				clip = playerComponent->shotClip;
			} else {
				clip = playerComponent->dryFireClip;
			}
			soundManager->playClipAtSource(clip, audioSourceComponent->sourceHandle);
		};
	eventManager->registerForEvent<ShotEvent>(shotCallback);

	std::function<void(const HealthChangedEvent& event)> healthChangedCallback =
		[world = &world, soundManager = &soundManager, healthLabel = gui.healthLabel](const HealthChangedEvent& event) {
			PlayerComponent* playerComponent = world->getComponent<PlayerComponent>(event.entity);

			std::stringstream sstream;
			sstream << event.newHealth;
			healthLabel->setText(sstream.str());

			if (event.healthChange < 0) {
				AudioSourceComponent* audioSourceComponent = world->getComponent<AudioSourceComponent>(event.entity);
				soundManager->playClipAtSource(playerComponent->hurtClip, audioSourceComponent->sourceHandle);
			}
		};
	eventManager->registerForEvent<HealthChangedEvent>(healthChangedCallback);

	std::function<void(const GemCountChangedEvent& event)> gemCountChangedCallback =
		[world = &world, soundManager = &soundManager, gemLabel = gui.gemLabel](const GemCountChangedEvent& event) {
			PlayerComponent* playerComponent = world->getComponent<PlayerComponent>(event.source);
			AudioSourceComponent* audioSourceComponent = world->getComponent<AudioSourceComponent>(event.source);

			std::stringstream sstream;
			sstream << event.newGemCount;
			gemLabel->setText(sstream.str());

			soundManager->playClipAtSource(playerComponent->gemPickupClip, audioSourceComponent->sourceHandle);
		};
	eventManager->registerForEvent<GemCountChangedEvent>(gemCountChangedCallback);

	std::function<void(const BulletCountChangedEvent& event)> bulletCountChangedCallback =
		[world = &world, soundManager = &soundManager, bulletLabel = gui.bulletLabel](const BulletCountChangedEvent& event) {
			PlayerComponent* playerComponent = world->getComponent<PlayerComponent>(event.source);
			AudioSourceComponent* audioSourceComponent = world->getComponent<AudioSourceComponent>(event.source);

			std::stringstream sstream;
			sstream << event.newBulletCount;
			bulletLabel->setText(sstream.str());
		};
	eventManager->registerForEvent<BulletCountChangedEvent>(bulletCountChangedCallback);

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
	renderer.draw();
	debugDrawer.draw();
	uiRenderer.draw();

	SDL_GL_SwapWindow(window);
}

void Game::update()
{
	input.update();

	spiderSpawner->update(timeDelta);

	/* AI/Input */
	playerInputSystem->update(timeDelta);
	followSystem->update(timeDelta);
	spiderSystem->update(timeDelta);

	/* Physics */
	rigidbodyMotorSystem->update(timeDelta);
	velocitySystem->update(timeDelta);
	shootingSystem->update(timeDelta);

	dynamicsWorld->stepSimulation(timeDelta);

	/* Display */
	playerFacingSystem->update(timeDelta);
	collisionUpdateSystem->update(timeDelta);
	cameraSystem->update(timeDelta);
	modelRenderSystem->update(timeDelta);
	pointLightSystem->update(timeDelta);
	audioSourceSystem->update(timeDelta);
	audioListenerSystem->update(timeDelta);

	renderer.update(timeDelta);
	soundManager.update();

	/* Cleanup */
	expiresSystem->update(timeDelta);

	world.cleanupEntities();
}

void Game::handleEvent(SDL_Event& event)
{
	if (!this->console->isVisible()) {
		input.handleEvent(event);
	}

	switch(event.type) {
	case SDL_QUIT:
		running = false;
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
			case SDLK_UP:
			case SDLK_DOWN:
				console->recallHistory(event.key.keysym.sym == SDLK_UP);
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
		patchData.model.material.setProperty("shininess", MaterialProperty(1000000.0f));

		Renderer::ModelHandle terrainModelHandle = renderer.getModelHandle(patchData.model);
		Renderer::RenderableHandle terrainHandle = renderer.getRenderableHandle(terrainModelHandle, shader);
		renderer.setRenderableTransform(terrainHandle, glm::translate(glm::mat4(), position));

		patchData.collision = patchData.patch.getCollisionData(glm::ivec2(), scale);
		patchData.vertArray = new btTriangleIndexVertexArray(patchData.collision.indices.size() / 3, patchData.collision.indices.data(), 3 * sizeof(unsigned), patchData.collision.vertices.size(), patchData.collision.vertices.data(), 3 * sizeof(float));
		patchData.shape = new btBvhTriangleMeshShape(patchData.vertArray, true);
		patchData.object = new btCollisionObject();
		patchData.object->setCollisionShape(patchData.shape);
		patchData.object->setWorldTransform(btTransform(Util::glmToBt(glm::quat()), Util::glmToBt(position)));
		dynamicsWorld->addCollisionObject(patchData.object);
	}
}

glm::vec3 roomBoxCenter(const RoomBox& box)
{
	return glm::vec3((box.left + box.right) / 2.0f, 0.0f, (box.bottom + box.top) / 2.0f);
}
