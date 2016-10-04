#pragma once

#include "Framework/Component.h"
#include "Framework/DefaultComponentConstructor.h"
#include "Game/Components/TransformComponent.h"
#include "Environment/Room.h"

class FollowComponent : public Component
{
public:
	FollowComponent() : repathTimer(0.0f), pathNode(0), enabled(true) { }

	struct Data
	{
		Data() : target(World::NullEntity), repathTime(3.0f), raycastStartOffset(0.0f) { }
		eid_t target;
		float repathTime;
		glm::vec3 raycastStartOffset;
	};

	Data data;
	bool enabled;
	float repathTimer;
	glm::vec3 lastPosition;
	std::vector<glm::vec3> path;
	unsigned pathNode;
};

class FollowConstructor : public DefaultComponentConstructor<FollowComponent> {
	using DefaultComponentConstructor<FollowComponent>::DefaultComponentConstructor;
};