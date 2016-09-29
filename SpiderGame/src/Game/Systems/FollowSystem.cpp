
#include "FollowSystem.h"

#include "Util.h"

#include "Game/Components/TransformComponent.h"
#include "Game/Components/FollowComponent.h"
#include "Game/Components/RigidbodyMotorComponent.h"
#include "Game/Components/CollisionComponent.h"

#include <cmath>
#include <algorithm>
#include <unordered_set>

FollowSystem::FollowSystem(World& world, btDynamicsWorld* dynamicsWorld, Room room)
	: System(world),
	dynamicsWorld(dynamicsWorld),
	room(room)
{
	require<TransformComponent>();
	require<FollowComponent>();
	require<RigidbodyMotorComponent>();
	require<CollisionComponent>();
}

void FollowSystem::updateEntity(float dt, eid_t entity)
{
	TransformComponent* transformComponent = world.getComponent<TransformComponent>(entity);
	FollowComponent* followComponent = world.getComponent<FollowComponent>(entity);
	RigidbodyMotorComponent* rigidbodyMotorComponent = world.getComponent<RigidbodyMotorComponent>(entity);
	CollisionComponent* collisionComponent = world.getComponent<CollisionComponent>(entity);

	if (followComponent->target == nullptr || !followComponent->enabled) {
		return;
	}

	followComponent->repathTimer += dt;

	Transform* finalTarget = followComponent->target.get();
	glm::vec3 localTarget(0.0f);
	bool pathFound = false;

	glm::vec3 from = transformComponent->transform->getWorldPosition() + followComponent->raycastStartOffset;
	glm::vec3 to = finalTarget->getWorldPosition();
	btVector3 btStart(Util::glmToBt(from));
	btVector3 btEnd(Util::glmToBt(to));

	float distanceToTarget = (btEnd - btStart).length();
	float distanceToHit = FLT_MAX;

	btConvexShape* convexShape = new btBoxShape(btVector3(0.65f, 0.2f, 0.5f));

	btQuaternion rotation = Util::glmToBt(transformComponent->transform->getWorldRotation()); 
	btTransform btTStart(rotation, btStart);
	btTransform btTEnd(rotation, btEnd);
	btCollisionWorld::ClosestConvexResultCallback sweepTestCallback(btStart, btEnd);
	sweepTestCallback.m_collisionFilterMask = CollisionGroupAll ^ (CollisionGroupPlayer | CollisionGroupEnemy);
	this->dynamicsWorld->convexSweepTest(convexShape, btTStart, btTEnd, sweepTestCallback);
	distanceToHit = (sweepTestCallback.m_hitPointWorld - btStart).length();

	// Bullet reports the hit point as very far away if no contact is found
	if (distanceToHit >= distanceToTarget - 0.1f) {
		localTarget = to;
		pathFound = true;
	} else {
		if (followComponent->repathTimer >= followComponent->repathTime) {
			// Try pathfinding again
			this->findPath(from, to, followComponent->path);
			followComponent->pathNode = 0;
			followComponent->repathTimer -= followComponent->repathTime;
		}

		if (followComponent->path.size() > 0 && followComponent->pathNode < followComponent->path.size()) {
			pathFound = true;
			localTarget = followComponent->path[followComponent->pathNode];
			if (glm::distance(glm::vec3(from.x, 0.0f, from.z), localTarget) <= 1.0f) {
				followComponent->pathNode++;
			}
		}
	}

	if (pathFound) {
		glm::vec3 dir = localTarget - from;
		float angle = atan2f(dir.x, dir.z);
		btQuaternion quat(btVector3(0.0f, 1.0f, 0.0f), angle);
		rigidbodyMotorComponent->facing = Util::btToGlm(quat);
		rigidbodyMotorComponent->movement = glm::vec2(-1.0f, 0.0f);
	} else {
		rigidbodyMotorComponent->movement = glm::vec2(0.0f, 0.0f);
	}
}

RoomPortal FollowSystem::getPortal(const RoomBox& box, int otherBoxIndex)
{
	for (unsigned i = 0; i < box.portals.size(); i++) {
		if (box.portals[i].otherBox == otherBoxIndex) {
			return box.portals[i];
		}
	}
	assert(false);
	return RoomPortal();
}

bool FollowSystem::findPath(const glm::vec3& start, const glm::vec3& finalTarget, std::vector<glm::vec3>& path)
{
	// Figure out where we are
	int startBox = room.boxForCoordinate(glm::vec2(start.x, start.z));
	int finishBox = room.boxForCoordinate(glm::vec2(finalTarget.x, finalTarget.z));

	path.clear();
	if (startBox < 0 || finishBox < 0) {
		return false;
	}

	std::vector<int> prevBox(room.boxes.size(), -1);
	std::vector<int> boxesToProcess;
	std::unordered_set<int> visitedBoxes;

	prevBox[startBox] = startBox;
	boxesToProcess.push_back(startBox);

	while (boxesToProcess.size() > 0) {
		int current = boxesToProcess.back();
		boxesToProcess.pop_back();

		if (current == finishBox) {
			break;
		}

		visitedBoxes.insert(current);
		std::vector<RoomPortal>& portals = room.boxes[current].portals;

		for (unsigned i = 0; i < portals.size(); i++) {
			int next = portals[i].otherBox;
			if (visitedBoxes.find(next) != visitedBoxes.end()) {
				continue;
			}

			boxesToProcess.push_back(next);
			prevBox[next] = current;
		}
	}

	if (prevBox[finishBox] == -1) {
		return false;
	}

	int current = finishBox;
	RoomBox currentBox = room.boxes[current];
	while (current != startBox) {
		int previous = prevBox[current];
		RoomPortal portal = this->getPortal(currentBox, previous);
		path.push_back(glm::vec3((portal.x0 + portal.x1) / 2.0f, 0.0f, (portal.y0 + portal.y1) / 2.0f));

		current = previous;
		currentBox = room.boxes[current];
	}

	std::reverse(path.begin(), path.end());
	return true;
}