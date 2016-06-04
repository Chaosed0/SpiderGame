#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <memory>

class Transform
{
public:
	Transform();
	Transform(glm::vec3 position);
	Transform(glm::vec3 position, glm::quat rotation);
	Transform(glm::vec3 position, glm::quat rotation, glm::vec3 scale);

	glm::vec3 getPosition() const;
	void setPosition(glm::vec3 newPosition);

	glm::quat getRotation() const;
	void setRotation(glm::quat newRotation);

	glm::vec3 getScale() const;
	void setScale(glm::vec3 newScale);

	std::shared_ptr<Transform> getParent() const;
	void setParent(const std::shared_ptr<Transform>& parent);

	glm::vec3 getForward() const;

	glm::mat4 matrix() const;

	static const Transform identity;
private:
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 scale;

	std::shared_ptr<Transform> parent;
};