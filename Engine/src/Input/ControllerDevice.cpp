
#include "Input/ControllerDevice.h"

#include <cassert>
#include <algorithm>

ControllerAxis axisFromSdl(SDL_GameControllerAxis axis, Sint16 value);
ControllerAxis axisFromSdl(SDL_GameControllerButton button);

ControllerDevice::ControllerDevice()
	: isConnected(false), controller(nullptr, SDL_GameControllerClose)
{ }

float ControllerDevice::getAxis(ControllerAxis axis, bool previous)
{
	auto iter = axisDataMap.find(axis);
	if (iter == axisDataMap.end()) {
		return 0.0f;
	}

	if (previous) {
		return iter->second.previousValue;
	}
	return iter->second.currentValue;
}

void ControllerDevice::update()
{
	for (auto iter = axisDataMap.begin(); iter != axisDataMap.end(); ++iter) {
		ControllerAxis axis = iter->first;
		AxisData& data = iter->second;
		data.previousValue = data.currentValue;
		data.currentValue = data.pendingValue;
	}

	// Read SDL axes
	for (int i = SDL_CONTROLLER_AXIS_LEFTX; i < SDL_CONTROLLER_AXIS_MAX; i++) {
		SDL_GameControllerAxis sdlAxis = (SDL_GameControllerAxis)(i);
		Sint16 sdlValue = SDL_GameControllerGetAxis(this->controller.get(), sdlAxis);
		float value = sdlValue / (float)INT16_MAX;

		// We invert the Y axes because SDL has down as positive, and that's dumb
		switch(sdlAxis) {
		case SDL_CONTROLLER_AXIS_LEFTX:
			axisDataMap[ControllerAxis_LStickXPos].currentValue = (std::max)(0.0f, value);
			axisDataMap[ControllerAxis_LStickXNeg].currentValue = (std::max)(0.0f, -value);
			break;
		case SDL_CONTROLLER_AXIS_LEFTY:
			axisDataMap[ControllerAxis_LStickYPos].currentValue = (std::max)(0.0f, -value);
			axisDataMap[ControllerAxis_LStickYNeg].currentValue = (std::max)(0.0f, value);
			break;
		case SDL_CONTROLLER_AXIS_RIGHTX:
			axisDataMap[ControllerAxis_RStickXPos].currentValue = (std::max)(0.0f, value);
			axisDataMap[ControllerAxis_RStickXNeg].currentValue = (std::max)(0.0f, -value);
			break;
		case SDL_CONTROLLER_AXIS_RIGHTY:
			axisDataMap[ControllerAxis_RStickYPos].currentValue = (std::max)(0.0f, -value);
			axisDataMap[ControllerAxis_RStickYNeg].currentValue = (std::max)(0.0f, value);
			break;
		case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
			axisDataMap[ControllerAxis_LeftTrigger].currentValue = value;
			break;
		case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
			axisDataMap[ControllerAxis_RightTrigger].currentValue = value;
			break;
		default:
			break;
		}
	}
}

SDL_JoystickID ControllerDevice::getID()
{
	return id;
}

SDL_JoystickGUID ControllerDevice::getGuid()
{
	SDL_Joystick* joystick = SDL_GameControllerGetJoystick(this->controller.get());
	return SDL_JoystickGetGUID(joystick);
}

bool ControllerDevice::connected()
{
	return this->isConnected;
}

void ControllerDevice::handleEvent(const SDL_Event& event)
{
	switch(event.type) {
	case SDL_CONTROLLERDEVICEADDED:
		this->open(event.cdevice.which);
		break;
	case SDL_CONTROLLERDEVICEREMOVED:
		assert(event.cdevice.which == this->id);
		this->close();
		break;
	case SDL_CONTROLLERBUTTONDOWN:
	case SDL_CONTROLLERBUTTONUP: {
		assert(event.cbutton.which == this->id);
		ControllerAxis axis = axisFromSdl((SDL_GameControllerButton)event.cbutton.button);
		axisDataMap[axis].pendingValue = (event.cbutton.state == SDL_PRESSED) ? 1.0f: 0.0f;
		break;
	}
	default:
		assert(false);
	}
}

void ControllerDevice::open(SDL_JoystickID id)
{
	this->id = id;
	this->controller = GameController(SDL_GameControllerOpen(id), SDL_GameControllerClose);
	SDL_Joystick* joystick = SDL_GameControllerGetJoystick(controller.get());

	if (SDL_JoystickIsHaptic(joystick)) {
		Haptic haptic(SDL_HapticOpenFromJoystick(joystick), SDL_HapticClose);
		if (SDL_HapticRumbleSupported(haptic.get())) {
			SDL_HapticRumbleInit(haptic.get());
		}
	}

	this->isConnected = true;
}

void ControllerDevice::close()
{
	this->controller = nullptr;
	this->isConnected = false;
	this->id = 0;
}

ControllerAxis axisFromSdl(SDL_GameControllerAxis axis, Sint16 value)
{
	ControllerAxis convertedAxis;
	switch (axis) {
	case SDL_CONTROLLER_AXIS_INVALID:
		convertedAxis = ControllerAxis_None; break;
	case SDL_CONTROLLER_AXIS_LEFTX:
		convertedAxis = (value > 0 ? ControllerAxis_LStickXPos : ControllerAxis_LStickXNeg); break;
	case SDL_CONTROLLER_AXIS_LEFTY:
		convertedAxis = (value > 0 ? ControllerAxis_LStickYPos : ControllerAxis_LStickYNeg); break;
	case SDL_CONTROLLER_AXIS_RIGHTX:
		convertedAxis = (value > 0 ? ControllerAxis_RStickXPos : ControllerAxis_RStickXNeg); break;
	case SDL_CONTROLLER_AXIS_RIGHTY:
		convertedAxis = (value > 0 ? ControllerAxis_RStickYPos : ControllerAxis_RStickYNeg); break;
	case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
		convertedAxis = ControllerAxis_LeftTrigger; break;
	case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
		convertedAxis = ControllerAxis_RightTrigger; break;
	default:
		convertedAxis = ControllerAxis_None; break;
	}

	return convertedAxis;
}

ControllerAxis axisFromSdl(SDL_GameControllerButton button)
{
	ControllerAxis convertedAxis;
	switch (button) {
	case SDL_CONTROLLER_BUTTON_INVALID:
		convertedAxis = ControllerAxis_None; break;
	case SDL_CONTROLLER_BUTTON_A:
		convertedAxis = ControllerAxis_A; break;
	case SDL_CONTROLLER_BUTTON_B:
		convertedAxis = ControllerAxis_B; break;
	case SDL_CONTROLLER_BUTTON_X:
		convertedAxis = ControllerAxis_X; break;
	case SDL_CONTROLLER_BUTTON_Y:
		convertedAxis = ControllerAxis_Y; break;
	case SDL_CONTROLLER_BUTTON_BACK:
		convertedAxis = ControllerAxis_Back; break;
	case SDL_CONTROLLER_BUTTON_GUIDE:
		convertedAxis = ControllerAxis_Guide; break;
	case SDL_CONTROLLER_BUTTON_START:
		convertedAxis = ControllerAxis_Start; break;
	case SDL_CONTROLLER_BUTTON_LEFTSTICK:
		convertedAxis = ControllerAxis_LStick; break;
	case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
		convertedAxis = ControllerAxis_RStick; break;
	case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
		convertedAxis = ControllerAxis_LeftShoulder; break;
	case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
		convertedAxis = ControllerAxis_RightShoulder; break;
	case SDL_CONTROLLER_BUTTON_DPAD_UP:
		convertedAxis = ControllerAxis_DPadUp; break;
	case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
		convertedAxis = ControllerAxis_DPadDown; break;
	case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
		convertedAxis = ControllerAxis_DPadLeft; break;
	case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
		convertedAxis = ControllerAxis_DPadRight; break;
	default:
		convertedAxis = ControllerAxis_None; break;
	}

	return convertedAxis;
}
