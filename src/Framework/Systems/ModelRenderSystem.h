#pragma once

#include "Framework/System.h"
#include "Renderer/Renderer.h"

class ModelRenderSystem : public System
{
public:
	ModelRenderSystem(World& world, Renderer& renderer);
	void updateEntity(float dt, eid_t entity);
private:
	Renderer& renderer;
};