#pragma once

#include <SDL.h>

#include <vector>
#include <string>

#include "KbmDevice.h"
#include "ControllerDevice.h"

#include "Optional.h"

enum Device
{
	Device_Kbm,
	Device_Controller0,
	Device_Controller1,
	Device_Controller2,
	Device_Controller3,
};

struct AxisProps {
	AxisProps(float sensitivity, float deadZone, float pressedThreshold) : sensitivity(sensitivity), deadZone(deadZone), pressedThreshold(pressedThreshold) { }
	AxisProps() : sensitivity(1.0f), deadZone(0.2f), pressedThreshold(0.4f) { }
	float sensitivity;
	float deadZone;
	float pressedThreshold;
};

template <typename T>
class InputMapper
{
public:
	struct Axis {
		Axis() : positive((T)(0)), negative((T)(0)) { }
		Axis(T positive, T negative, AxisProps props) : positive(positive), negative(negative), props(props) { }
		T positive;
		T negative;
		AxisProps props;
	};

	Axis getMappingFor(const std::string& action) const;
	void setMapping(const std::string& action, Axis axis, bool isDefault=false);
	void setAxisProps(const std::string& action, AxisProps props, bool isDefault=false);
private:
	std::unordered_map<std::string, Axis> defaultInputMap;
	std::unordered_map<std::string, Axis> inputMap;
};

class Input
{
public:
	Input();
	void initialize();
	void update();
	void handleEvent(const SDL_Event& event);

	float getAxis(const std::string& axis, Device device);
	bool getButtonDown(const std::string& button, Device device);
	bool getButtonUp(const std::string& button, Device device);
	bool getButton(const std::string& button, Device device);

	void setDefaultMapping(const std::string& action, KbmAxis positive, KbmAxis negative, AxisProps props = AxisProps());
	void setDefaultMapping(const std::string& action, ControllerAxis positive, ControllerAxis negative, AxisProps props = AxisProps());
	void setMapping(const std::string& action, KbmAxis positive, KbmAxis negative, AxisProps props = AxisProps());
	void setMapping(const std::string& action, ControllerAxis positive, ControllerAxis negative, int index, AxisProps props = AxisProps());

	void setSensitivity(const std::string& action, float sensitivity, Device device);
	void setDeadZone(const std::string& action, float deadZone, Device device);
	void setPressedThreshold(const std::string& action, float pressedThreshold, Device device);
private:
	KbmDevice kbmDevice;
	std::vector<ControllerDevice> controllerDevices;

	InputMapper<KbmAxis> kbmMapping;
	std::vector<InputMapper<ControllerAxis>> controllerMappings;

	std::experimental::optional<std::reference_wrapper<ControllerDevice>> getControllerDevice(SDL_JoystickID id);
	void handleDeviceAdd(const SDL_Event& event);
	void handleDeviceEvent(const SDL_Event& event, SDL_JoystickID id);
	InputMapper<KbmAxis>::Axis kbmMappingFor(const std::string& axis);
	InputMapper<ControllerAxis>::Axis controllerMappingFor(const std::string& axis, int index);

	AxisProps getAxisProperties(const std::string& action, Device device);
	float SimulateAxis(const std::string& action, Device device, bool previous = false);
};

template <typename T>
typename InputMapper<T>::Axis InputMapper<T>::getMappingFor(const std::string& axis) const
{
	auto iter = inputMap.find(axis);
	if (iter != inputMap.end()) {
		return iter->second;
	}

	auto defaultIter = defaultInputMap.find(axis);
	if (defaultIter != defaultInputMap.end()) {
		return defaultIter->second;
	}

	throw "Tried to get mapping for nonexistent axis " + axis;
}

template <typename T>
void InputMapper<T>::setMapping(const std::string& action, Axis axis, bool isDefault=false)
{
	if (isDefault) {
		defaultInputMap[action] = axis;
	} else {
		inputMap[action] = axis;
	}
}

template <typename T>
void InputMapper<T>::setAxisProps(const std::string& action, AxisProps props, bool isDefault=false)
{
	if (isDefault) {
		auto iter = defaultInputMap.find(action);
		if (iter == defaultInputMap.end()) {
			return;
		}
		iter->second.props = props;
	} else {
		auto iter = inputMap.find(action);
		if (iter == inputMap.end()) {
			return;
		}
		iter->second.props = props;
	}
}