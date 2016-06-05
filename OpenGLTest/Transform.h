#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <memory>
#include <vector>

class Transform
{
public:
	Transform();
	Transform(glm::vec3 position);
	Transform(glm::vec3 position, glm::quat rotation);
	Transform(glm::vec3 position, glm::quat rotation, glm::vec3 scale);
	~Transform();

	glm::vec3 getPosition() const;
	void setPosition(glm::vec3 newPosition);

	glm::quat getRotation() const;
	void setRotation(glm::quat newRotation);

	glm::vec3 getScale() const;
	void setScale(glm::vec3 newScale);

	void addChild(Transform* child);
	void removeChild(Transform* child);

	glm::vec3 getForward() const;

	glm::mat4 matrix() const;

	static const Transform identity;
private:
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 scale;

	Transform* parent;
	std::vector<Transform*> children;

	void setParent(Transform* parent);
};