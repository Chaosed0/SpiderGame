#pragma once

#include <SDL.h>
#include <btBulletDynamicsCommon.h>

#include <random>
#include <vector>
#include <memory>

#include "Renderer/Renderer.h"
#include "Renderer/ShaderLoader.h"
#include "Renderer/ModelLoader.h"
#include "Renderer/Box.h"
#include "Renderer/Model.h"

#include "Renderer/UI/UIRenderer.h"
#include "Renderer/UI/UIQuad.h"
#include "Renderer/UI/Label.h"
#include "Renderer/UI/Font.h"

#include "Sound/SoundManager.h"

#include "Environment/Room.h"
#include "Environment/MeshBuilder.h"

#include "Framework/World.h"
#include "Framework/Prefab.h"
#include "Framework/EventManager.h"

#include "Game/Responders/DamageEventResponder.h"
#include "Game/Responders/PlayerJumpResponder.h"
#include "Game/Responders/HurtboxPlayerResponder.h"
#include "Game/Events/RestartEvent.h"

struct RoomData
{
	Room room;
	btRigidBody* rigidBody;
	MeshBuilder meshBuilder;
};

struct GUI
{
	std::shared_ptr<Label> healthLabel;
	std::shared_ptr<Label> bulletLabel;
	std::shared_ptr<UIQuad> healthImage;
	std::shared_ptr<UIQuad> bulletImage;
	std::shared_ptr<UIQuad> redGemImage;
	std::shared_ptr<UIQuad> greenGemImage;
	std::shared_ptr<UIQuad> blueGemImage;
	std::shared_ptr<Label> facingLabel;
	std::shared_ptr<UIQuad> reticleImage;

	UIRenderer::UIElementHandle healthLabelHandle;
	UIRenderer::UIElementHandle healthImageHandle;
	UIRenderer::UIElementHandle bulletLabelHandle;
	UIRenderer::UIElementHandle bulletImageHandle;
	UIRenderer::UIElementHandle redGemImageHandle;
	UIRenderer::UIElementHandle greenGemImageHandle;
	UIRenderer::UIElementHandle blueGemImageHandle;
	UIRenderer::UIElementHandle facingLabelHandle;
	UIRenderer::UIElementHandle reticleHandle;
};

struct SceneInfo
{
	World* world;
	Renderer* renderer;
	UIRenderer* uiRenderer;
	SoundManager* soundManager;
	EventManager* eventManager;
	btDynamicsWorld* dynamicsWorld;
	std::default_random_engine* generator;

	int windowWidth, windowHeight;
};

class Scene
{
public:
	Scene(const SceneInfo& info);
	void setup();
private:
	void setupPrefabs();

	ModelLoader modelLoader;
	TextureLoader textureLoader;
	ShaderLoader shaderLoader;

	GUI gui;

	RoomGenerator roomGenerator;
	RoomData roomData;

	World& world;
	Renderer& renderer;
	UIRenderer& uiRenderer;
	SoundManager& soundManager;
	EventManager& eventManager;
	btDynamicsWorld* dynamicsWorld;
	std::default_random_engine& generator;

	int windowWidth;
	int windowHeight;

	std::shared_ptr<DamageEventResponder> damageEventResponder;
	std::shared_ptr<PlayerJumpResponder> playerJumpResponder;
	std::shared_ptr<HurtboxPlayerResponder> hurtboxPlayerResponder;

	bool prefabsSetup;
	Prefab pedestalPrefab;
	Prefab barrelPrefab;
	Prefab bulletPrefab;
	Prefab tablePrefab;
	Prefab bulletTracer;
	Prefab muzzleFlash;
	Prefab spiderPrefab;
	Prefab spiderSpawnerPrefab;
	Prefab playerGunPrefab;
	Prefab playerPrefab;
	Prefab cameraPrefab;
	Prefab playerLightPrefab;

	Prefab platformPrefab;

	std::vector<Prefab> gemPrefabs;
	std::vector<Prefab> gemLightPrefabs;

	Shader shader;
	Shader skinnedShader;
	Shader singleColorShader;
	Shader textShader;
	Shader imageShader;
};
