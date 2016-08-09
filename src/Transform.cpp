
#include "Transform.h"

const Transform Transform::identity = Transform();

Transform::Transform()
	: Transform(glm::vec3(0,0,0), glm::quat(), glm::vec3(1,1,1))
{ }

Transform::Transform(glm::vec3 position)
	: Transform(position, glm::quat(), glm::vec3(1, 1, 1))
{ }

Transform::Transform(glm::vec3 position, glm::quat rotation)
	: Transform(position, rotation, glm::vec3(1, 1, 1))
{ }

Transform::Transform(glm::vec3 position, glm::quat rotation, glm::vec3 scale)
{
	setPosition(position);
	setRotation(rotation);
	setScale(scale);

	parent = nullptr;
}

Transform::~Transform()
{
	for (unsigned int i = 0; i < children.size(); i++) {
		children[i]->setParent(nullptr);
	}

	if (parent != nullptr) {
		parent->removeChild(this);
	}
}

glm::vec3 Transform::getPosition() const
{
	if (parent != nullptr) {
		return parent->getPosition() + position;
	}
	return position;
}

void Transform::setPosition(glm::vec3 newPosition)
{
	position = newPosition;
}

glm::quat Transform::getRotation() const
{
	if (parent != nullptr) {
		return parent->getRotation() * rotation;
	}
	return rotation;
}

void Transform::setRotation(glm::quat newRotation)
{
	rotation = glm::normalize(newRotation);
}

glm::vec3 Transform::getScale() const
{
	if (parent != nullptr) {
		return parent->getScale() * scale;
	}
	return scale;
}

void Transform::setScale(glm::vec3 newScale)
{
	scale = newScale;
}

glm::vec3 Transform::getForward() const
{
	return rotation * glm::vec3(0,0,1);
}

void Transform::addChild(Transform* child)
{
	children.push_back(child);
	child->setParent(this);
}

void Transform::removeChild(Transform* child)
{
	for (unsigned int i = 0; i < children.size(); i++) {
		if (children[i] == child) {
			children[i] = children[children.size()-1];
			children.pop_back();
		}
	}
}

void Transform::setParent(Transform* parent)
{
	assert(parent != this);
	this->parent = parent;
}

glm::mat4 Transform::matrix() const
{
	glm::mat4 posMatrix = glm::mat4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		position.x, position.y, position.z, 1.0f
		);
	glm::mat4 rotMatrix(glm::toMat4(rotation));
	glm::mat4 scaleMatrix = glm::mat4(
		scale.x, 0.0f, 0.0f, 0.0f,
		0.0f, scale.y, 0.0f, 0.0f,
		0.0f, 0.0f, scale.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
		);
	glm::mat4 parentMatrix = (this->parent == nullptr ? glm::mat4() : this->parent->matrix());

	return parentMatrix * (posMatrix * rotMatrix * scaleMatrix);
}