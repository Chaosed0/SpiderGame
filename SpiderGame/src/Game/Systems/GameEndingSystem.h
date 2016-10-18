#pragma once

#include "Framework/EventManager.h"
#include "Framework/System.h"
#include "Framework/World.h"

#include "Renderer/UI/UIQuad.h"
#include "Sound/SoundManager.h"

#include "Game/Events/GemCountChangedEvent.h"

enum GameEndState
{
	GameEndState_NotEnded,
	GameEndState_DefendingGems,
	GameEndState_Blackout,
	GameEndState_Fadein,
	GameEndState_Rest,
	GameEndState_Finished
};

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

	float timer;
	bool allGemsPlaced;
	GameEndState state;

	void onGemCountChanged(const GemCountChangedEvent& gemCountChangedEvent);
};