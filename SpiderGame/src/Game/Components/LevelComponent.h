#pragma once

#include "Framework/Component.h"
#include "Framework/DefaultComponentConstructor.h"

#include "Environment/Room.h"

class LevelComponent : public Component
{
public:
	struct Data {
		Data() { }
		Data(Room room) : room(room) { }
		Room room;
	};

	Data data;
};

class LevelConstructor : public DefaultComponentConstructor<LevelComponent> {
	using DefaultComponentConstructor<LevelComponent>::DefaultComponentConstructor;
};