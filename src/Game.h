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
#include "Framework/Entity.h"

#include "Console/Console.h"
#include "Environment/Terrain.h"
#include "Environment/Room.h"

#include "Framework/Systems/ModelRenderSystem.h"
#include "Framework/Systems/CollisionUpdateSystem.h"
#include "Framework/Systems/CameraSystem.h"
#include "Framework/Systems/RigidbodyMotorSystem.h"
#include "Framework/Systems/PlayerInputSystem.h"

struct GameTerrainData
{
	Model model;
	TerrainPatch patch;
	TerrainPatchCollision collision;
	btTriangleIndexVertexArray* vertArray;
	btBvhTriangleMeshShape* shape;
	btCollisionObject* object;
};

struct RoomData
{
	std::vector<btCollisionObject*> collisionObjects;
};

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

	RoomData roomData;
	std::vector<GameTerrainData> terrainData;
	std::vector<Transform> pointLightTransforms;
	Model pointLightModel;
	Model skyboxModel;

	Entity player;
	Entity camera;
	btRigidBody* playerBody;

	std::vector<Entity> entities;
	std::unique_ptr<ModelRenderSystem> modelRenderSystem;
	std::unique_ptr<CollisionUpdateSystem> collisionUpdateSystem;
	std::unique_ptr<CameraSystem> cameraSystem;
	std::unique_ptr<RigidbodyMotorSystem> rigidbodyMotorSystem;
	std::unique_ptr<PlayerInputSystem> playerInputSystem;

	btDiscreteDynamicsWorld* dynamicsWorld;
	BulletDebugDrawer debugDrawer;

	std::unique_ptr<Console> console;
	bool consoleIsVisible;

	std::default_random_engine generator;

	void exit();
	void setWireframe(bool on);
	void setNoclip(bool on);
};