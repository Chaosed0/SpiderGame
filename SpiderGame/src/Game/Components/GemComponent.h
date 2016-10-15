#pragma once

#include "Framework/Component.h"
#include "Framework/DefaultComponentConstructor.h"

enum GemState {
	GemState_OnPedestal,
	GemState_OnSlab
};

enum GemColor {
	GemColor_Red = 0,
	GemColor_Green,
	GemColor_Blue,
	GemColor_Unknown
};

class GemComponent : public Component
{
public:
	GemComponent() : light(World::NullEntity), pulseTimer(0.0f) { }

	struct Data {
		Data(GemColor color, GemState state, const glm::vec3& lightColor, float pulseTime, const PointLight& minIntensity, const PointLight& maxIntensity)
			: color(color), state(state), lightColor(lightColor), pulseTime(pulseTime), minIntensity(minIntensity), maxIntensity(maxIntensity) { }
		Data() : Data(GemColor_Unknown, GemState_OnPedestal, glm::vec3(0.0f), 3.0f, PointLight(), PointLight()) { }

		GemColor color;
		GemState state;

		glm::vec3 lightColor;
		float pulseTime;
		PointLight minIntensity;
		PointLight maxIntensity;
	};

	Data data;
	eid_t light;
	float pulseTimer;
};

class GemConstructor : public DefaultComponentConstructor<GemComponent> {
	using DefaultComponentConstructor<GemComponent>::DefaultComponentConstructor;
};