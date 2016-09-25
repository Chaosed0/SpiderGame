#pragma once

#include <SDL.h>

#include <memory>
#include <unordered_map>

enum ControllerAxis
{
	ControllerAxis_None = 0,
	ControllerAxis_A,
	ControllerAxis_B,
	ControllerAxis_X,
	ControllerAxis_Y,
	ControllerAxis_Back,
	ControllerAxis_Guide,
	ControllerAxis_Start,
	ControllerAxis_LStick,
	ControllerAxis_RStick,
	ControllerAxis_DPadUp,
	ControllerAxis_DPadDown,
	ControllerAxis_DPadLeft,
	ControllerAxis_DPadRight,
	ControllerAxis_LeftShoulder,
	ControllerAxis_RightShoulder,

	/* Real axes */
	ControllerAxis_LStickXPos,
	ControllerAxis_LStickXNeg,
	ControllerAxis_LStickYPos,
	ControllerAxis_LStickYNeg,
	ControllerAxis_RStickXPos,
	ControllerAxis_RStickXNeg,
	ControllerAxis_RStickYPos,
	ControllerAxis_RStickYNeg,
	ControllerAxis_LeftTrigger,
	ControllerAxis_RightTrigger,
};

class ControllerDevice
{
public:
	ControllerDevice();
	void update();
	void handleEvent(const SDL_Event& event);

	float getAxis(ControllerAxis axis, bool previous = false);

	SDL_JoystickID getID();
	SDL_JoystickGUID getGuid();
	bool connected();

	void open(SDL_JoystickID id);
	void close();
private:

	typedef std::unique_ptr<SDL_Haptic, decltype(&SDL_HapticClose)> Haptic;
	typedef std::unique_ptr<SDL_GameController, decltype(&SDL_GameControllerClose)> GameController;

	struct AxisData {
		AxisData() : currentValue(0.0f), previousValue(0.0f) { }
		float pendingValue;
		float currentValue;
		float previousValue;
	};

	bool isConnected;
	GameController controller;
	SDL_JoystickID id;
	std::unordered_map<ControllerAxis, AxisData> axisDataMap;
};