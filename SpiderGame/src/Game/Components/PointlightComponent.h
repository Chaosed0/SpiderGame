#pragma once

#include "Framework/Component.h"
#include "Framework/DefaultComponentConstructor.h"
#include "Renderer/Renderer.h"

struct PointLightComponent : public Component
{
	struct Data {
		Data() : Data(-1) { }
		Data(int index) : pointLightIndex(index) { }
		int pointLightIndex;
	};

	Data data;
};

class PointLightConstructor : public DefaultComponentConstructor<PointLightComponent> {
	using DefaultComponentConstructor<PointLightComponent>::DefaultComponentConstructor;
};
