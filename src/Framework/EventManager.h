#pragma once

#include <unordered_map>
#include <functional>

#include "Event.h"

typedef uint32_t eventid_t;

class EventManager
{
public:
	EventManager() : nextEventId(0) { }

	template <class T>
	void sendEvent(const T& event);

	template <class T>
	uint32_t registerForEvent(std::function<void(const T&)> eventListener);
private:
	template <class T>
	eventid_t registerEventType();

	typedef std::function<void(const Event* event)> EventCallback;
	typedef std::vector<EventCallback> EventCallbackList;

	std::unordered_map<size_t, eventid_t> eventTypeMap;
	std::vector<EventCallbackList> eventListeners;
	eventid_t nextEventId;
};

template <class T>
void EventManager::sendEvent(const T& event)
{
	auto iter = this->eventTypeMap.find(typeid(T).hash_code());
	if (iter == this->eventTypeMap.end()) {
		return;
	}

	EventCallbackList& list = eventListeners[iter->second];
	for (EventCallback callback : list) {
		callback(&event);
	}
}

template <class T>
uint32_t EventManager::registerForEvent(std::function<void(const T&)> eventListener)
{
	eventid_t eventid;
	auto iter = this->eventTypeMap.find(typeid(T).hash_code());
	if (iter == this->eventTypeMap.end()) {
		eventid = this->registerEventType<T>();
	} else {
		eventid = iter->second;
	}

	EventCallbackList& list = eventListeners[eventid];
	EventCallback callback([eventListener](const Event* event) {
		eventListener(*(static_cast<const T*>(event)));
	});
	list.push_back(callback);

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