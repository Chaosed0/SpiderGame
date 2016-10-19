#pragma once

#include "Framework/Component.h"
#include "Framework/DefaultComponentConstructor.h"
#include "Renderer/Camera.h"

class CameraComponent : public Component
{
public:
	CameraComponent() : isActive(true) { }
	typedef Camera Data;
	Camera data;
	bool isActive;
};

class CameraConstructor : public DefaultComponentConstructor<CameraComponent> {
	using DefaultComponentConstructor<CameraComponent>::DefaultComponentConstructor;
};