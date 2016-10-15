#pragma once

#include "Framework/System.h"
#include "Renderer/Renderer.h"

class GemSystem : public System
{
public:
	GemSystem(World& world, Renderer& renderer);
	virtual void updateEntity(float dt, eid_t entity);
private:
	Renderer& renderer;
};
