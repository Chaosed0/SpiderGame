#pragma once

#include "Framework/Component.h"
#include "Framework/DefaultComponentConstructor.h"
#include "Renderer/Renderer.h"

struct PointLightComponent : public Component
{
	struct Data {
		Data() : pointLightIndex(-1) { }
		int pointLightIndex;
	};

	Data data;
};

class PointLightConstructor : public DefaultComponentConstructor<PointLightComponent> {
	using DefaultComponentConstructor<PointLightComponent>::DefaultComponentConstructor;
};
