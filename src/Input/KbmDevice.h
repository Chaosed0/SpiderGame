#pragma once

#include <SDL.h>

#include <unordered_map>

enum KbmAxis
{
	KbmAxis_None = 0,

	/* Keyboard */
	KbmAxis_Shift,
	KbmAxis_Alt,
	KbmAxis_Command,
	KbmAxis_Control,
	KbmAxis_LeftShift,
	KbmAxis_LeftAlt,
	KbmAxis_LeftCommand,
	KbmAxis_LeftControl,
	KbmAxis_RightShift,
	KbmAxis_RightAlt,
	KbmAxis_RightCommand,
	KbmAxis_RightControl,
	KbmAxis_Escape,
	KbmAxis_F1,
	KbmAxis_F2,
	KbmAxis_F3,
	KbmAxis_F4,
	KbmAxis_F5,
	KbmAxis_F6,
	KbmAxis_F7,
	KbmAxis_F8,
	KbmAxis_F9,
	KbmAxis_F10,
	KbmAxis_F11,
	KbmAxis_F12,
	KbmAxis_Key0,
	KbmAxis_Key1,
	KbmAxis_Key2,
	KbmAxis_Key3,
	KbmAxis_Key4,
	KbmAxis_Key5,
	KbmAxis_Key6,
	KbmAxis_Key7,
	KbmAxis_Key8,
	KbmAxis_Key9,
	KbmAxis_A,
	KbmAxis_B,
	KbmAxis_C,
	KbmAxis_D,
	KbmAxis_E,
	KbmAxis_F,
	KbmAxis_G,
	KbmAxis_H,
	KbmAxis_I,
	KbmAxis_J,
	KbmAxis_K,
	KbmAxis_L,
	KbmAxis_M,
	KbmAxis_N,
	KbmAxis_O,
	KbmAxis_P,
	KbmAxis_Q,
	KbmAxis_R,
	KbmAxis_S,
	KbmAxis_T,
	KbmAxis_U,
	KbmAxis_V,
	KbmAxis_W,
	KbmAxis_X,
	KbmAxis_Y,
	KbmAxis_Z,
	KbmAxis_Backquote,
	KbmAxis_Minus,
	KbmAxis_Equals,
	KbmAxis_Backspace,
	KbmAxis_Tab,
	KbmAxis_LeftBracket,
	KbmAxis_RightBracket,
	KbmAxis_Backslash,
	KbmAxis_Semicolon,
	KbmAxis_Quote,
	KbmAxis_Return,
	KbmAxis_Comma,
	KbmAxis_Period,
	KbmAxis_Slash,
	KbmAxis_Space,
	KbmAxis_Insert,
	KbmAxis_Delete,
	KbmAxis_Home,
	KbmAxis_End,
	KbmAxis_PageUp,
	KbmAxis_PageDown,
	KbmAxis_LeftArrow,
	KbmAxis_RightArrow,
	KbmAxis_UpArrow,
	KbmAxis_DownArrow,
	KbmAxis_Pad0,
	KbmAxis_Pad1,
	KbmAxis_Pad2,
	KbmAxis_Pad3,
	KbmAxis_Pad4,
	KbmAxis_Pad5,
	KbmAxis_Pad6,
	KbmAxis_Pad7,
	KbmAxis_Pad8,
	KbmAxis_Pad9,
	KbmAxis_Numlock,
	KbmAxis_PadDivide,
	KbmAxis_PadMultiply,
	KbmAxis_PadMinus,
	KbmAxis_PadPlus,
	KbmAxis_PadEnter,
	KbmAxis_PadPeriod,
	KbmAxis_Clear,
	KbmAxis_PadEquals,
	KbmAxis_F13,
	KbmAxis_F14,
	KbmAxis_F15,

	/* Mouse buttons */
	KbmAxis_MouseLeft,
	KbmAxis_MouseRight,
	KbmAxis_MouseMiddle,
	KbmAxis_MouseButton4,
	KbmAxis_MouseButton5,
	KbmAxis_MouseButton6,
	KbmAxis_MouseButton7,
	KbmAxis_MouseButton8,
	KbmAxis_MouseButton9,

	/* Mouse Axes */
	KbmAxis_MouseXPos,
	KbmAxis_MouseXNeg,
	KbmAxis_MouseYPos,
	KbmAxis_MouseYNeg,
	KbmAxis_MouseScrollXPos,
	KbmAxis_MouseScrollXNeg,
	KbmAxis_MouseScrollYPos,
	KbmAxis_MouseScrollYNeg,
};

class KbmDevice {
public:
	KbmDevice();
	void update();
	void handleEvent(const SDL_Event& event);

	float getAxis(KbmAxis axis, bool previous = false);
private:
	struct AxisData {
		AxisData() : currentValue(0.0f), previousValue(0.0f) { }
		float currentValue;
		float previousValue;
	};

	std::unordered_map<KbmAxis, AxisData> axisDataMap;
};