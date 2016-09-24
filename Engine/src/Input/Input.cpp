
#include "Input/Input.h"

#include <algorithm>

Input::Input()
	: controllerDevices(4), controllerMappings(4)
{ }

void Input::initialize() {
	int numControllers = SDL_NumJoysticks();
	for (SDL_JoystickID i = 0; i < std::min(numControllers, 4); i++) {
		controllerDevices[i].open(i);
	}
}

void Input::update() {
	kbmDevice.update();
	for (unsigned i = 0; i < controllerDevices.size(); i++) {
		controllerDevices[i].update();
	}
}

void Input::handleEvent(const SDL_Event& event)
{
	switch (event.type) {
	case SDL_CONTROLLERDEVICEADDED:
		this->handleDeviceAdd(event);
		break;
	case SDL_CONTROLLERDEVICEREMOVED:
		this->handleDeviceEvent(event, event.cdevice.which);
		break;
	case SDL_CONTROLLERBUTTONDOWN:
	case SDL_CONTROLLERBUTTONUP:
		this->handleDeviceEvent(event, event.cbutton.which);
		break;
	case SDL_KEYDOWN:
	case SDL_KEYUP:
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
	case SDL_MOUSEMOTION:
	case SDL_MOUSEWHEEL:
		kbmDevice.handleEvent(event);
		break;
	default:
		break;
	}
}

float Input::getAxis(const std::string& axisName, Device device)
{
	return SimulateAxis(axisName, device);
}

bool Input::getButtonDown(const std::string& button, Device device)
{
	AxisProps props = getAxisProperties(button, device);
	float currentValue = SimulateAxis(button, device, false);
	float previousValue = SimulateAxis(button, device, true);
	return currentValue > props.pressedThreshold && previousValue < props.pressedThreshold;
}

bool Input::getButtonUp(const std::string& button, Device device)
{
	AxisProps props = getAxisProperties(button, device);
	float currentValue = SimulateAxis(button, device, false);
	float previousValue = SimulateAxis(button, device, true);
	return currentValue < props.pressedThreshold && previousValue > props.pressedThreshold;
}

bool Input::getButton(const std::string& button, Device device)
{
	AxisProps props = getAxisProperties(button, device);
	float currentValue = SimulateAxis(button, device, false);
	return currentValue > props.pressedThreshold;
}

AxisProps Input::getAxisProperties(const std::string& action, Device device)
{
	if (device == Device_Kbm) {
		InputMapper<KbmAxis>::Axis axis = kbmMappingFor(action);
		return axis.props;
	} else {
		unsigned index = device - Device_Controller0;
		if (index < controllerDevices.size()) {
			InputMapper<ControllerAxis>::Axis axis = controllerMappingFor(action, index);
			return axis.props;
		}
	}

	return AxisProps();
}

float Input::SimulateAxis(const std::string& action, Device device, bool previous)
{
	float rawAxisValue;
	AxisProps axisProps;
	if (device == Device_Kbm) {
		InputMapper<KbmAxis>::Axis axis = kbmMappingFor(action);
		rawAxisValue = kbmDevice.getAxis(axis.positive, previous) -
			kbmDevice.getAxis(axis.negative, previous);
		axisProps = axis.props;
	} else {
		unsigned index = device - Device_Controller0;
		if (index < controllerDevices.size()) {
			InputMapper<ControllerAxis>::Axis axis = controllerMappingFor(action, index);
			rawAxisValue = controllerDevices[index].getAxis(axis.positive, previous) -
				controllerDevices[index].getAxis(axis.negative, previous);
			axisProps = axis.props;
		}
	}

	if (std::abs(rawAxisValue) < axisProps.deadZone) {
		return 0.0f;
	}
	return axisProps.sensitivity * rawAxisValue;
}

InputMapper<KbmAxis>::Axis Input::kbmMappingFor(const std::string& axis)
{
	return kbmMapping.getMappingFor(axis);
}

InputMapper<ControllerAxis>::Axis Input::controllerMappingFor(const std::string& axis, int index)
{
	if (index < 0 || (unsigned)index >= controllerMappings.size()) {
		return InputMapper<ControllerAxis>::Axis(ControllerAxis_None, ControllerAxis_None, AxisProps());
	}

	InputMapper<ControllerAxis>& controllerMapping = controllerMappings[index];
	return controllerMapping.getMappingFor(axis);
}

void Input::setDefaultMapping(const std::string& action, KbmAxis positive, KbmAxis negative, AxisProps props)
{
	kbmMapping.setMapping(action, InputMapper<KbmAxis>::Axis(positive, negative, props), true);
}

void Input::setDefaultMapping(const std::string& action, ControllerAxis positive, ControllerAxis negative, AxisProps props)
{
	for (unsigned i = 0; i < controllerMappings.size(); i++) {
		controllerMappings[i].setMapping(action, InputMapper<ControllerAxis>::Axis(positive, negative, props), true);
	}
}

void Input::setMapping(const std::string& action, KbmAxis positive, KbmAxis negative, AxisProps props)
{
	kbmMapping.setMapping(action, InputMapper<KbmAxis>::Axis(positive, negative, props), false);
}

void Input::setMapping(const std::string& action, ControllerAxis positive, ControllerAxis negative, int index, AxisProps props)
{
	if (index < 0 || (unsigned)index >= controllerMappings.size()) {
		return;
	}
	controllerMappings[index].setMapping(action, InputMapper<ControllerAxis>::Axis(positive, negative, props), true);
}

void Input::setSensitivity(const std::string& action, float sensitivity, Device device)
{
}

void Input::setDeadZone(const std::string& action, float deadZone, Device device)
{
}

void Input::setPressedThreshold(const std::string& action, float pressedThreshold, Device device)
{
}

std::experimental::optional<std::reference_wrapper<ControllerDevice>> Input::getControllerDevice(SDL_JoystickID id)
{
	for (unsigned i = 0; i < controllerDevices.size(); i++) {
		if (controllerDevices[i].getID() == id) {
			return controllerDevices[i];
		}
	}

	return std::experimental::optional<std::reference_wrapper<ControllerDevice>>();
}

void Input::handleDeviceAdd(const SDL_Event& event)
{
	assert(event.type == SDL_CONTROLLERDEVICEADDED);

	// Find first unconnected device
	for (unsigned i = 0; i < controllerDevices.size(); i++) {
		if (!controllerDevices[i].connected()) {
			controllerDevices[i].handleEvent(event);
		}
	}

	return;
}

void Input::handleDeviceEvent(const SDL_Event& event, SDL_JoystickID id)
{
	std::experimental::optional<std::reference_wrapper<ControllerDevice>> deviceOpt = this->getControllerDevice(id);
	if (deviceOpt) {
		ControllerDevice& device = *deviceOpt;
		device.handleEvent(event);
	}
}
