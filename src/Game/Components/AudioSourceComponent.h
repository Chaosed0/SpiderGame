#pragma once

#include "Framework/Component.h"
#include "Sound/SoundManager.h"

struct AudioSourceComponent : public Component
{
	AudioSourceComponent() : sourceHandle(nullptr) { }
	SoundManager::SourceHandle sourceHandle;
};