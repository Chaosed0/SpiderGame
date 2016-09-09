#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <memory>
#include <vector>

#include "Optional.h"

class Transform
{
public:
	Transform();
	Transform(glm::vec3 position);
	Transform(glm::vec3 position, glm::quat rotation);
	Transform(glm::vec3 position, glm::quat rotation, glm::vec3 scale);

	glm::vec3 getPosition() const;
	glm::vec3 getWorldPosition() const;
	void setPosition(glm::vec3 newPosition);

	glm::quat getRotation() const;
	glm::quat getWorldRotation() const;
	void setRotation(glm::quat newRotation);

	glm::vec3 getScale() const;
	glm::vec3 getWorldScale() const;
	void setScale(glm::vec3 newScale);

	glm::vec3 getForward() const;
	glm::vec3 getWorldForward() const;

	glm::mat4 matrix() const;
	glm::mat4 matrix();

	void setParent(const std::shared_ptr<Transform>& parent);

	void* userData;
	static const Transform identity;
private:
	glm::mat4 getParentMat4() const;
	const std::shared_ptr<Transform> getParentInternal() const;

	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 scale;

	glm::mat4 cacheMatrix;
	bool dirty;

	std::experimental::optional<std::weak_ptr<Transform>> parent;
	glm::mat4 toMat4() const;

	static const glm::mat4 identityMat4;
	static const std::shared_ptr<Transform> identityPtr;
};