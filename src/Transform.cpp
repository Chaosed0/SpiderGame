
#include "Transform.h"
#include "Util.h"

const Transform Transform::identity = Transform();
const std::shared_ptr<Transform> Transform::identityPtr(new Transform());
const glm::mat4 Transform::identityMat4 = glm::mat4();

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
	: dirty(true)
{
	this->setPosition(position);
	this->setRotation(rotation);
	this->setScale(scale);
}

glm::vec3 Transform::getPosition() const
{
	return position;
}

glm::vec3 Transform::getWorldPosition() const
{
	return this->getParentInternal()->getPosition() + this->position;
}

void Transform::setPosition(glm::vec3 newPosition)
{
	this->position = newPosition;
	this->dirty = true;
}

glm::quat Transform::getRotation() const
{
	return this->rotation;
}

glm::quat Transform::getWorldRotation() const
{
	return this->getParentInternal()->getRotation() * this->rotation;
}

void Transform::setRotation(glm::quat newRotation)
{
	this->rotation = glm::normalize(newRotation);
	this->dirty = true;
}

glm::vec3 Transform::getScale() const
{
	return scale;
}

glm::vec3 Transform::getWorldScale() const
{
	return this->getParentInternal()->getScale() * this->scale;
}

void Transform::setScale(glm::vec3 newScale)
{
	this->scale = newScale;
	this->dirty = true;
}

glm::vec3 Transform::getForward() const
{
	return getRotation() * Util::forward;
}

glm::vec3 Transform::getWorldForward() const
{
	return getWorldRotation() * Util::forward;
}

void Transform::setParent(const std::shared_ptr<Transform>& parent)
{
	this->parent.emplace(std::weak_ptr<Transform>(parent));
}

glm::mat4 Transform::matrix() const
{
	if (!this->dirty) {
		return this->getParentMat4() * this->cacheMatrix;
	} else {
		return this->getParentMat4() * this->toMat4();
	}
}

glm::mat4 Transform::matrix()
{
	if (this->dirty) {
		this->cacheMatrix = toMat4();
		this->dirty = false;
	}
	
	return this->getParentMat4() * this->cacheMatrix;
}

glm::mat4 Transform::toMat4() const
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

	return posMatrix * rotMatrix * scaleMatrix;
}

glm::mat4 Transform::getParentMat4() const
{
	if (this->parent && !this->parent->expired()) {
		return this->parent.value().lock()->matrix();
	} else {
		return identityMat4;
	}
}

const std::shared_ptr<Transform> Transform::getParentInternal() const
{
	if (this->parent && !this->parent->expired()) {
		return this->parent.value().lock();
	} else {
		return identityPtr;
	}
}