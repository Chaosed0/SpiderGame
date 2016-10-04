#pragma once

#include "Framework/Component.h"
#include "Framework/DefaultComponentConstructor.h"
#include "Renderer/Camera.h"

class CameraComponent : public Component
{
public:
	typedef Camera Data;
	Camera data;
};

class CameraConstructor : public DefaultComponentConstructor<CameraComponent> {
	using DefaultComponentConstructor<CameraComponent>::DefaultComponentConstructor;
};