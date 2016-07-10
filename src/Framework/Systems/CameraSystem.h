#pragma once

#include "Framework/System.h"
#include "Renderer/Renderer.h"

class CameraSystem : public System
{
public:
	CameraSystem(World& world, Renderer& renderer);
	void updateEntity(float dt, eid_t entity);
private:
	Renderer& renderer;
};