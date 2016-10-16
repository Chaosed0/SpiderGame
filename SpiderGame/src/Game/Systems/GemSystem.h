#pragma once

#include "Framework/System.h"
#include "Framework/EventManager.h"
#include "Renderer/Renderer.h"

#include "Game/Events/GemCountChangedEvent.h"

class GemSystem : public System
{
public:
	GemSystem(World& world, Renderer& renderer, EventManager& eventManager);
	virtual void updateEntity(float dt, eid_t entity);
private:
	Renderer& renderer;
	bool allGemsPlaced;

	void onGemCountChanged(const GemCountChangedEvent& gemCountChangedEvent);

	static const float endGemHeight;
	static const float endGemAngularSpeed;
	static const float airLiftTime;
	static const float endGameTime;
};
