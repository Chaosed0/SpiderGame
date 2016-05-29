#pragma once

#include "System.h"
#include "Renderer.h"

class ModelRenderSystem : public System
{
public:
	ModelRenderSystem(Renderer& renderer);
	void updateEntity(float dt, Entity& entity);
private:
	Renderer& renderer;
};