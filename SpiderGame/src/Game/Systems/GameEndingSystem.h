#pragma once

#include "Framework/EventManager.h"
#include "Framework/System.h"
#include "Framework/World.h"

#include "Renderer/UI/UIQuad.h"
#include "Sound/SoundManager.h"

#include "Game/Events/GemCountChangedEvent.h"
#include "Framework/CollisionEvent.h"

class GameEndingSystem : public System
{
public:
	GameEndingSystem(World& world, EventManager& eventManager, SoundManager& soundManager);

	virtual void updateEntity(float dt, eid_t entity);
private:
	EventManager& eventManager;
	SoundManager& soundManager;

	void onGemCountChanged(const GemCountChangedEvent& gemCountChangedEvent);
	void onGemLightOn(const GemLightOnEvent& gemLightOnEvent);
	void onCollision(const CollisionEvent& collisionEvent);
};