#pragma once

#include "Framework/Component.h"
#include "Game/Components/TransformComponent.h"
#include "Environment/Room.h"

class FollowComponent : public Component
{
public:
	FollowComponent() : target(World::NullEntity), repathTime(3.0f), repathTimer(repathTime), raycastStartOffset(0.0f), pathNode(0), enabled(true) { }

	eid_t target;
	bool enabled;
	float repathTime;
	float repathTimer;
	glm::vec3 lastPosition;

	std::vector<glm::vec3> path;
	unsigned pathNode;

	glm::vec3 raycastStartOffset;
};