#pragma once

#include "Framework/Component.h"
#include "Framework/DefaultComponentConstructor.h"

struct AudioListenerComponent : public Component
{
	struct Data {
		Data() : gain(1.0f) { }
		float gain;
	};
	Data data;
};

class AudioListenerConstructor : public DefaultComponentConstructor<AudioListenerComponent> {
	using DefaultComponentConstructor<AudioListenerComponent>::DefaultComponentConstructor;
};