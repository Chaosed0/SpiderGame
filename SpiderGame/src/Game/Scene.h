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
#include "Framework/EventManager.h"

#include "Game/Responders/DamageEventResponder.h"
#include "Game/Responders/PlayerJumpResponder.h"
#include "Game/Responders/HurtboxPlayerResponder.h"

struct RoomData
{
	Room room;
	btRigidBody* rigidBody;
	MeshBuilder meshBuilder;
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
	std::shared_ptr<UIQuad> reticleImage;

	UIRenderer::UIElementHandle healthLabelHandle;
	UIRenderer::UIElementHandle healthImageHandle;
	UIRenderer::UIElementHandle gemLabelHandle;
	UIRenderer::UIElementHandle gemImageHandle;
	UIRenderer::UIElementHandle bulletLabelHandle;
	UIRenderer::UIElementHandle bulletImageHandle;
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
};

class Scene
{
public:
	Scene(const SceneInfo& info);
	void setup();
private:

	ModelLoader modelLoader;

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

	std::shared_ptr<DamageEventResponder> damageEventResponder;
	std::shared_ptr<PlayerJumpResponder> playerJumpResponder;
	std::shared_ptr<HurtboxPlayerResponder> hurtboxPlayerResponder;
};
