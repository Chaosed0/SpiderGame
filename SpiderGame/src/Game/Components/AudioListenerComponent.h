#pragma once

#include "Framework/Component.h"

struct AudioListenerComponent : public Component
{
	AudioListenerComponent() : volume(1.0f) { }
	float volume;
};