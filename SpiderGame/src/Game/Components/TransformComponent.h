#pragma once

#include "Framework/Component.h"
#include "Framework/DefaultComponentConstructor.h"
#include "Transform.h"

#include "Game/Extra/PrefabConstructionInfo.h"

struct TransformComponent : public Component
{
	TransformComponent(const Transform& transform) : data(new Transform(transform)) { }
	TransformComponent() : data(new Transform()) { }
	std::shared_ptr<Transform> data;
};

class TransformConstructor : public ComponentConstructor {
public:
	TransformConstructor() { }
	TransformConstructor(const Transform& transform) : transform(transform) { }

	virtual ComponentConstructorInfo construct(World& world, eid_t parent, void* userinfo) const
	{
		PrefabConstructionInfo* constructionInfo = (PrefabConstructionInfo*)userinfo;

		TransformComponent* component = new TransformComponent(transform);
		TransformComponent* parentComponent = world.getComponent<TransformComponent>(parent);

		if (constructionInfo != nullptr) {
			*component->data = constructionInfo->initialTransform;
		}

		if (parent != World::NullEntity) {
			component->data->setParent(parentComponent->data);
		}

		return ComponentConstructorInfo(component, typeid(TransformComponent).hash_code());
	}
private:
	Transform transform;
};