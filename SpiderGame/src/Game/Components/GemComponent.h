#pragma once

#include "Framework/Component.h"
#include "Framework/DefaultComponentConstructor.h"

enum GemState {
	GemState_OnPedestal,
	GemState_OnSlab,
	GemState_ShouldFree,
	GemState_Free
};

enum GemColor {
	GemColor_Red = 0,
	GemColor_Green,
	GemColor_Blue,
	GemColor_Unknown
};

enum GemLightState {
	GemLightState_Unset,
	GemLightState_Small,
	GemLightState_Max,
	GemLightState_Deleted
};

class GemComponent : public Component
{
public:
	GemComponent() : light(World::NullEntity), endGameTimer(0.0f) { }

	struct Data {
		Data(GemColor color, GemState state, const glm::vec3& lightColor, float pulseTime, const PointLight& minIntensity, const PointLight& maxIntensity)
			: color(color), state(state), lightColor(lightColor), minIntensity(minIntensity), maxIntensity(maxIntensity), lightState(GemLightState_Unset) { }
		Data() : Data(GemColor_Unknown, GemState_OnPedestal, glm::vec3(0.0f), 3.0f, PointLight(), PointLight()) { }

		GemColor color;
		GemState state;
		GemLightState lightState;

		glm::vec3 lightColor;
		PointLight minIntensity;
		PointLight maxIntensity;
	};

	Data data;
	eid_t light;
	float endGameTimer;
};

class GemConstructor : public DefaultComponentConstructor<GemComponent> {
	using DefaultComponentConstructor<GemComponent>::DefaultComponentConstructor;
};