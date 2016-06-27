#pragma once

#include "Framework/System.h"
#include "Renderer/Renderer.h"

class CameraSystem : public System
{
public:
	CameraSystem(Renderer& renderer);
	void updateEntity(float dt, Entity& entity);
private:
	Renderer& renderer;
};