#pragma once

#include "Framework/Component.h"
#include "Renderer/Camera.h"

class CameraComponent : public Component
{
public:
	Camera camera;
};