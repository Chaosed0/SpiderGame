#pragma once

#include <SDL.h>
#include <btBulletDynamicsCommon.h>

#include <random>
#include <vector>
#include <memory>

#include "Renderer/Renderer.h"
#include "Renderer/BulletDebugDrawer.h"
#include "Renderer/ShaderLoader.h"
#include "Renderer/ModelLoader.h"
#include "Renderer/Box.h"
#include "Renderer/Model.h"

#include "Renderer/UI/UIRenderer.h"

#include "Sound/SoundManager.h"

#include "Console/Console.h"
#include "Environment/Terrain.h"
#include "Environment/Room.h"
#include "Environment/MeshBuilder.h"

#include "Framework/World.h"
#include "Game/Systems/ShootingSystem.h"
#include "Game/Systems/ModelRenderSystem.h"
#include "Game/Systems/CollisionUpdateSystem.h"
#include "Game/Systems/CameraSystem.h"
#include "Game/Systems/RigidbodyMotorSystem.h"
#include "Game/Systems/PlayerInputSystem.h"
#include "Game/Systems/FollowSystem.h"
#include "Game/Systems/SpiderSystem.h"
#include "Game/Systems/ExpiresSystem.h"
#include "Game/Systems/VelocitySystem.h"
#include "Game/Systems/PlayerFacingSystem.h"
#include "Game/Systems/AudioListenerSystem.h"
#include "Game/Systems/AudioSourceSystem.h"
#include "Game/Systems/PointLightSystem.h"
#include "Game/Systems/SpawnerSystem.h"
#include "Game/Systems/PlayerDeathSystem.h"

#include "Framework/Physics.h"
#include "Framework/EventManager.h"

#include "Input/Input.h"
#include "Scene.h"

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

	bool wireframe;
	bool running;
	bool restart;
	Uint32 lastUpdate;
	float accumulator;
	float timeDelta;

	SDL_Window* window;
	SDL_GLContext context;

	std::unique_ptr<ShootingSystem> shootingSystem;
	std::unique_ptr<ModelRenderSystem> modelRenderSystem;
	std::unique_ptr<CollisionUpdateSystem> collisionUpdateSystem;
	std::unique_ptr<CameraSystem> cameraSystem;
	std::unique_ptr<RigidbodyMotorSystem> rigidbodyMotorSystem;
	std::unique_ptr<PlayerInputSystem> playerInputSystem;
	std::unique_ptr<FollowSystem> followSystem;
	std::unique_ptr<SpiderSystem> spiderSystem;
	std::unique_ptr<ExpiresSystem> expiresSystem;
	std::unique_ptr<VelocitySystem> velocitySystem;
	std::unique_ptr<PlayerFacingSystem> playerFacingSystem;
	std::unique_ptr<AudioListenerSystem> audioListenerSystem;
	std::unique_ptr<AudioSourceSystem> audioSourceSystem;
	std::unique_ptr<PointLightSystem> pointLightSystem;
	std::unique_ptr<SpawnerSystem> spawnerSystem;
	std::unique_ptr<PlayerDeathSystem> playerDeathSystem;

	std::unique_ptr<EventManager> eventManager;

	BulletDebugDrawer debugDrawer;
	std::unique_ptr<Console> console;

	std::unique_ptr<Physics> physics;
	World world;
	UIRenderer uiRenderer;
	Renderer renderer;
	SoundManager soundManager;
	btDiscreteDynamicsWorld* dynamicsWorld;
	std::default_random_engine generator;
	Input input;

	std::unique_ptr<Scene> scene;

	void exit();
	void setWireframe(bool on);
	void setNoclip(bool on);
	void setBulletDebugDraw(bool on);
	void refreshBulletDebugDraw();
	void restartGame();
};
