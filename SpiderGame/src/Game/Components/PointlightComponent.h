#pragma once

#include "Framework/Component.h"
#include "Renderer/Renderer.h"

struct PointLightComponent : public Component
{
	PointLightComponent() : pointLightIndex(-1) { }
	int pointLightIndex;
};