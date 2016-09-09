#pragma once

#include "Framework/System.h"

class Renderer;

class PointLightSystem : public System
{
public:
	PointLightSystem(World& world, Renderer& renderer);
	void updateEntity(float dt, eid_t entity);
private:
	Renderer& renderer;
};