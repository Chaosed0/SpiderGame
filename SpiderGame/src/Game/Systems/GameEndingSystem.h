#pragma once

#include "Framework/EventManager.h"
#include "Framework/System.h"
#include "Framework/World.h"

#include "Renderer/UI/UIQuad.h"
#include "Sound/SoundManager.h"

#include "Game/Events/GemCountChangedEvent.h"

class GameEndingSystem : public System
{
public:
	GameEndingSystem(World& world, EventManager& eventManager, SoundManager& soundManager);

	virtual void updateEntity(float dt, eid_t entity);

	static const float screenShakeTime;
	static const float gemDefenseTime;
	static const float blackoutTime;
	static const float fadeInTime;
	static const float endRestTime;
private:
	EventManager& eventManager;
	SoundManager& soundManager;

	void onGemCountChanged(const GemCountChangedEvent& gemCountChangedEvent);
};