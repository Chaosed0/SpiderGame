#pragma once

#include <unordered_map>
#include <functional>

#include "Event.h"
#include "ComponentBitmask.h"
#include "World.h"

typedef uint32_t eventid_t;

class EventManager
{
public:
	EventManager(const World& world) : world(world), nextEventId(0) { }

	template <class T>
	void sendEvent(const T& event);

	template <class T>
	uint32_t registerForEvent(std::function<void(const T&)> eventListener, ComponentBitmask requiredComponents);
private:
	template <class T>
	eventid_t registerEventType();

	typedef std::function<void(const Event* event)> EventCallbackInternal;
	struct EventStorage {
		ComponentBitmask requiredComponents;
		EventCallbackInternal callback;
	};
	typedef std::vector<EventStorage> EventCallbackList;

	std::unordered_map<size_t, eventid_t> eventTypeMap;
	std::vector<EventCallbackList> eventListeners;
	eventid_t nextEventId;
	const World& world;
};

template <class T>
void EventManager::sendEvent(const T& event)
{
	auto iter = this->eventTypeMap.find(typeid(T).hash_code());
	if (iter == this->eventTypeMap.end()) {
		return;
	}

	EventCallbackList& list = eventListeners[iter->second];
	for (EventStorage& eventStorage : list) {
		if (world.getEntityBitmask(event.target).hasComponents(eventStorage.requiredComponents)) {
			eventStorage.callback(&event);
		}
	}
}

template <class T>
uint32_t EventManager::registerForEvent(std::function<void(const T&)> eventListener, ComponentBitmask requiredComponents)
{
	eventid_t eventid;
	auto iter = this->eventTypeMap.find(typeid(T).hash_code());
	if (iter == this->eventTypeMap.end()) {
		eventid = this->registerEventType<T>();
	} else {
		eventid = iter->second;
	}

	EventCallbackList& list = eventListeners[eventid];
	EventStorage eventStorage;
	eventStorage.callback = [eventListener](const Event* event) {
		eventListener(*(static_cast<const T*>(event)));
	};
	eventStorage.requiredComponents = requiredComponents;
	list.push_back(eventStorage);

	return list.size()-1;
}

template <class T>
eventid_t EventManager::registerEventType()
{
	size_t hash = typeid(T).hash_code();
	eventTypeMap.emplace(hash, nextEventId);
	eventListeners.push_back(EventCallbackList());
	return nextEventId++;
}