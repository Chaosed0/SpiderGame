
#include "System.h"

bool System::shouldUpdate(const Entity& entity) const
{
	for (unsigned int i = 0; i < this->requiredComponents.size(); i++) {
		if (!entity.hasComponent(this->requiredComponents[i])) {
			return false;
		}
	}
	return true;
}

void System::update(float dt, std::vector<Entity>& entities)
{
	for (unsigned int i = 0; i < entities.size(); i++) {
		if (this->shouldUpdate(entities.at(i))) {
			updateEntity(dt, entities.at(i));
		}
	}
}
