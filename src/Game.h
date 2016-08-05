#pragma once

#include <SDL.h>
#include <btBulletDynamicsCommon.h>

#include <random>
#include <vector>
#include <memory>

#include "Renderer/BulletDebugDrawer.h"
#include "Renderer/Shader.h"
#include "Renderer/Renderer.h"
#include "Renderer/ModelLoader.h"
#include "Renderer/Box.h"
#include "Renderer/Model.h"

#include "Console/Console.h"
#include "Environment/Terrain.h"
#include "Environment/Room.h"
#include "Environment/MeshBuilder.h"

#include "Framework/World.h"
#include "Framework/Systems/ShootingSystem.h"
#include "Framework/Systems/ModelRenderSystem.h"
#include "Framework/Systems/CollisionUpdateSystem.h"
#include "Framework/Systems/CameraSystem.h"
#include "Framework/Systems/RigidbodyMotorSystem.h"
#include "Framework/Systems/PlayerInputSystem.h"
#include "Framework/Systems/FollowSystem.h"
#include "Framework/Systems/SpiderSystem.h"
#include "Framework/Systems/ExpiresSystem.h"

#include "Framework/Physics/Physics.h"
#include "Framework/Physics/Responders/PlayerJumpResponder.h"
#include "Framework/Physics/Responders/HurtboxPlayerResponder.h"

#include "Framework/EventManager.h"
#include "Framework/Responders/DamageEventResponder.h"

struct RoomData
{
	Room room;
	btRigidBody* rigidBody;
	MeshBuilder meshBuilder;
};

struct GameTerrainData
{
	Model model;
	TerrainPatch patch;
	TerrainPatchCollision collision;
	btTriangleIndexVertexArray* vertArray;
	btBvhTriangleMeshShape* shape;
	btCollisionObject* object;
};

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

	void generateTestTerrain();

	bool wireframe;
	bool running;
	Uint32 lastUpdate;
	float accumulator;
	float timeDelta;

	SDL_Window* window;
	SDL_GLContext context;

	Renderer renderer;
	ModelLoader modelLoader;

	Shader shader;
	Shader skinnedShader;
	Shader lightShader;
	Shader skyboxShader;
	Shader textShader;

	RoomGenerator roomGenerator;
	RoomData roomData;
	std::vector<GameTerrainData> terrainData;
	std::vector<Transform> pointLightTransforms;
	Model pointLightModel;
	Model skyboxModel;

	eid_t player;
	eid_t camera;
	btRigidBody* playerBody;

	World world;
	std::unique_ptr<ShootingSystem> shootingSystem;
	std::unique_ptr<ModelRenderSystem> modelRenderSystem;
	std::unique_ptr<CollisionUpdateSystem> collisionUpdateSystem;
	std::unique_ptr<CameraSystem> cameraSystem;
	std::unique_ptr<RigidbodyMotorSystem> rigidbodyMotorSystem;
	std::unique_ptr<PlayerInputSystem> playerInputSystem;
	std::unique_ptr<FollowSystem> followSystem;
	std::unique_ptr<SpiderSystem> spiderSystem;
	std::unique_ptr<ExpiresSystem> expiresSystem;

	std::unique_ptr<Physics> physics;
	std::shared_ptr<PlayerJumpResponder> playerJumpResponder;
	std::shared_ptr<HurtboxPlayerResponder> hurtboxPlayerResponder;

	EventManager eventManager;
	std::shared_ptr<DamageEventResponder> damageEventResponder;

	btDiscreteDynamicsWorld* dynamicsWorld;
	BulletDebugDrawer debugDrawer;

	std::unique_ptr<Console> console;
	bool consoleIsVisible;

	std::default_random_engine generator;

	void exit();
	void setWireframe(bool on);
	void setNoclip(bool on);
};
