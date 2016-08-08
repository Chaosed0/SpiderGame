#pragma once

#include "Framework/System.h"

class Renderer;

class CameraSystem : public System
{
public:
	CameraSystem(World& world);
	void updateEntity(float dt, eid_t entity);
private:
};