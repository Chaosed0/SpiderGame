#pragma once

#include <SDL.h>
#include <btBulletDynamicsCommon.h>

#include <random>
#include <vector>
#include <memory>

#include "Renderer/Renderer.h"
#include "Renderer/BulletDebugDrawer.h"
#include "Renderer/Shader.h"
#include "Renderer/ModelLoader.h"
#include "Renderer/Box.h"
#include "Renderer/Model.h"

#include "Renderer/UI/UIRenderer.h"
#include "Renderer/UI/Label.h"
#include "Renderer/UI/Font.h"

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

#include "Framework/Physics.h"
#include "Framework/EventManager.h"
#include "Game/Responders/DamageEventResponder.h"
#include "Game/Responders/PlayerJumpResponder.h"
#include "Game/Responders/HurtboxPlayerResponder.h"

#include "Input/Input.h"

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

struct GUI
{
	std::shared_ptr<Label> healthLabel;
	std::shared_ptr<Label> gemLabel;
	std::shared_ptr<Label> bulletLabel;
	std::shared_ptr<UIQuad> healthImage;
	std::shared_ptr<UIQuad> gemImage;
	std::shared_ptr<UIQuad> bulletImage;
	std::shared_ptr<Label> facingLabel;

	UIRenderer::UIElementHandle healthLabelHandle;
	UIRenderer::UIElementHandle healthImageHandle;
	UIRenderer::UIElementHandle gemLabelHandle;
	UIRenderer::UIElementHandle gemImageHandle;
	UIRenderer::UIElementHandle bulletLabelHandle;
	UIRenderer::UIElementHandle bulletImageHandle;
	UIRenderer::UIElementHandle facingLabelHandle;
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

	UIRenderer uiRenderer;
	GUI gui;

	SoundManager soundManager;

	Input input;

	Shader shader;
	Shader skinnedShader;
	Shader lightShader;
	Shader skyboxShader;
	Shader textShader;
	Shader imageShader;
	Shader backShader;

	RoomGenerator roomGenerator;
	RoomData roomData;
	std::vector<GameTerrainData> terrainData;
	std::vector<Transform> pointLightTransforms;
	Model pointLightModel;
	Model skyboxModel;

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
	std::unique_ptr<VelocitySystem> velocitySystem;
	std::unique_ptr<PlayerFacingSystem> playerFacingSystem;
	std::unique_ptr<AudioListenerSystem> audioListenerSystem;
	std::unique_ptr<AudioSourceSystem> audioSourceSystem;
	std::unique_ptr<PointLightSystem> pointLightSystem;

	std::unique_ptr<Physics> physics;

	std::unique_ptr<EventManager> eventManager;
	std::shared_ptr<DamageEventResponder> damageEventResponder;
	std::shared_ptr<PlayerJumpResponder> playerJumpResponder;
	std::shared_ptr<HurtboxPlayerResponder> hurtboxPlayerResponder;

	btDiscreteDynamicsWorld* dynamicsWorld;
	BulletDebugDrawer debugDrawer;

	std::unique_ptr<Console> console;

	std::default_random_engine generator;

	void exit();
	void setWireframe(bool on);
	void setNoclip(bool on);
	void setBulletDebugDraw(bool on);
	void refreshBulletDebugDraw();
};
