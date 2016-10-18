#pragma once

#include "Framework/System.h"
#include "Framework/EventManager.h"
#include "Renderer/Renderer.h"

#include "Game/Events/EndGameEvent.h"

class GemSystem : public System
{
public:
	GemSystem(World& world, Renderer& renderer, EventManager& eventManager);
	virtual void updateEntity(float dt, eid_t entity);
private:
	Renderer& renderer;
	EventManager& eventManager;
	bool allGemsPlaced;

	float endGameTime;

	void onAllGemsCollected(const AllGemsCollectedEvent& allGemsCollectedEvent);

	static const float endGemHeight;
	static const float endGemAngularSpeed;
	static const float airLiftTime;
};
