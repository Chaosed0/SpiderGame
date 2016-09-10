
#include "KbmDevice.h"

KbmAxis axisFromSdl(SDL_Keycode keycode);
KbmAxis altAxisFromSdl(SDL_Keycode keycode);
KbmAxis axisFromSdl(Uint8 mouseButton);

KbmDevice::KbmDevice()
{ }

void KbmDevice::update()
{
	for (auto iter = axisDataMap.begin(); iter != axisDataMap.end(); ++iter) {
		AxisData& data = iter->second;
		data.previousValue = data.currentValue;
		data.currentValue = data.pendingValue;

		if (iter->first >= KbmAxis_MouseXPos && iter->first <= KbmAxis_MouseScrollYNeg) {
			data.pendingValue = 0.0f;
		}
	}
}

void KbmDevice::handleEvent(const SDL_Event& event)
{
	switch (event.type) {
	case SDL_KEYDOWN:
	case SDL_KEYUP: {
		KbmAxis axis = axisFromSdl(event.key.keysym.sym);
		if (axis != KbmAxis_None) {
			axisDataMap[axis].pendingValue = (event.key.state == SDL_PRESSED ? 1.0f : 0.0f);
		}
		axis = altAxisFromSdl(event.key.keysym.sym);
		if (axis != KbmAxis_None) {
			axisDataMap[axis].pendingValue = (event.key.state == SDL_PRESSED ? 1.0f : 0.0f);
		}
		break;
	}
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP: {
		KbmAxis axis = axisFromSdl(event.button.button);
		if (axis != KbmAxis_None) {
			axisDataMap[axis].pendingValue = (event.button.state == SDL_PRESSED ? 1.0f : 0.0f);
		}
		break;
	}
	case SDL_MOUSEMOTION:
		// Invert the Y axis because down is positive, and we want up to be positive
		axisDataMap[KbmAxis_MouseXPos].pendingValue += (event.motion.xrel > 0.0f ? event.motion.xrel : 0.0f);
		axisDataMap[KbmAxis_MouseXNeg].pendingValue -= (event.motion.xrel < 0.0f ? event.motion.xrel : 0.0f);
		axisDataMap[KbmAxis_MouseYPos].pendingValue -= (event.motion.yrel < 0.0f ? event.motion.yrel : 0.0f);
		axisDataMap[KbmAxis_MouseYNeg].pendingValue += (event.motion.yrel > 0.0f ? event.motion.yrel : 0.0f);
		break;
	case SDL_MOUSEWHEEL:
		axisDataMap[KbmAxis_MouseScrollXPos].pendingValue += (event.wheel.x > 0.0f ? event.wheel.x : 0.0f);
		axisDataMap[KbmAxis_MouseScrollXNeg].pendingValue -= (event.wheel.x < 0.0f ? event.wheel.x : 0.0f);
		axisDataMap[KbmAxis_MouseScrollYPos].pendingValue += (event.wheel.y > 0.0f ? event.wheel.y : 0.0f);
		axisDataMap[KbmAxis_MouseScrollYNeg].pendingValue -= (event.wheel.y < 0.0f ? event.wheel.y : 0.0f);
		break;
	}
}

float KbmDevice::getAxis(KbmAxis axis, bool previous)
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

KbmAxis altAxisFromSdl(SDL_Keycode keycode)
{
	KbmAxis axis;
	switch (keycode) {
	case SDLK_LSHIFT:
	case SDLK_RSHIFT:
		axis = KbmAxis_Shift; break;
	case SDLK_LALT:
	case SDLK_RALT:
		axis = KbmAxis_Alt; break;
	case SDLK_LGUI:
	case SDLK_RGUI:
		axis = KbmAxis_Command; break;
	case SDLK_RCTRL:
	case SDLK_LCTRL:
		axis = KbmAxis_Control; break;
	default:
		axis = KbmAxis_None; break;
	}
	return axis;
}

KbmAxis axisFromSdl(SDL_Keycode keycode)
{
	KbmAxis axis;
	switch (keycode) {
	case SDLK_LSHIFT:
		axis = KbmAxis_LeftShift; break;
	case SDLK_RSHIFT:
		axis = KbmAxis_RightShift; break;
	case SDLK_LALT:
		axis = KbmAxis_LeftAlt; break;
	case SDLK_RALT:
		axis = KbmAxis_RightAlt; break;
	case SDLK_LGUI:
		axis = KbmAxis_LeftCommand; break;
	case SDLK_RGUI:
		axis = KbmAxis_RightCommand; break;
	case SDLK_RCTRL:
		axis = KbmAxis_LeftControl; break;
	case SDLK_LCTRL:
		axis = KbmAxis_RightControl; break;
	case SDLK_ESCAPE:
		axis = KbmAxis_Escape; break;
	case SDLK_F1:
		axis = KbmAxis_F1; break;
	case SDLK_F2:
		axis = KbmAxis_F2; break;
	case SDLK_F3:
		axis = KbmAxis_F3; break;
	case SDLK_F4:
		axis = KbmAxis_F4; break;
	case SDLK_F5:
		axis = KbmAxis_F5; break;
	case SDLK_F6:
		axis = KbmAxis_F6; break;
	case SDLK_F7:
		axis = KbmAxis_F7; break;
	case SDLK_F8:
		axis = KbmAxis_F8; break;
	case SDLK_F9:
		axis = KbmAxis_F9; break;
	case SDLK_F10:
		axis = KbmAxis_F10; break;
	case SDLK_F11:
		axis = KbmAxis_F11; break;
	case SDLK_F12:
		axis = KbmAxis_F12; break;
	case SDLK_0:
		axis = KbmAxis_Key0; break;
	case SDLK_1:
		axis = KbmAxis_Key1; break;
	case SDLK_2:
		axis = KbmAxis_Key2; break;
	case SDLK_3:
		axis = KbmAxis_Key3; break;
	case SDLK_4:
		axis = KbmAxis_Key4; break;
	case SDLK_5:
		axis = KbmAxis_Key5; break;
	case SDLK_6:
		axis = KbmAxis_Key6; break;
	case SDLK_7:
		axis = KbmAxis_Key7; break;
	case SDLK_8:
		axis = KbmAxis_Key8; break;
	case SDLK_9:
		axis = KbmAxis_Key9; break;
	case SDLK_a:
		axis = KbmAxis_A; break;
	case SDLK_b:
		axis = KbmAxis_B; break;
	case SDLK_c:
		axis = KbmAxis_C; break;
	case SDLK_d:
		axis = KbmAxis_D; break;
	case SDLK_e:
		axis = KbmAxis_E; break;
	case SDLK_f:
		axis = KbmAxis_F; break;
	case SDLK_g:
		axis = KbmAxis_G; break;
	case SDLK_h:
		axis = KbmAxis_H; break;
	case SDLK_i:
		axis = KbmAxis_I; break;
	case SDLK_j:
		axis = KbmAxis_J; break;
	case SDLK_k:
		axis = KbmAxis_K; break;
	case SDLK_l:
		axis = KbmAxis_L; break;
	case SDLK_m:
		axis = KbmAxis_M; break;
	case SDLK_n:
		axis = KbmAxis_N; break;
	case SDLK_o:
		axis = KbmAxis_O; break;
	case SDLK_p:
		axis = KbmAxis_P; break;
	case SDLK_q:
		axis = KbmAxis_Q; break;
	case SDLK_r:
		axis = KbmAxis_R; break;
	case SDLK_s:
		axis = KbmAxis_S; break;
	case SDLK_t:
		axis = KbmAxis_T; break;
	case SDLK_u:
		axis = KbmAxis_U; break;
	case SDLK_v:
		axis = KbmAxis_V; break;
	case SDLK_w:
		axis = KbmAxis_W; break;
	case SDLK_x:
		axis = KbmAxis_X; break;
	case SDLK_y:
		axis = KbmAxis_Y; break;
	case SDLK_z:
		axis = KbmAxis_Z; break;
	case SDLK_BACKQUOTE:
		axis = KbmAxis_Backquote; break;
	case SDLK_MINUS:
		axis = KbmAxis_Minus; break;
	case SDLK_EQUALS:
		axis = KbmAxis_Equals; break;
	case SDLK_BACKSPACE:
		axis = KbmAxis_Backspace; break;
	case SDLK_TAB:
		axis = KbmAxis_Tab; break;
	case SDLK_LEFTBRACKET:
		axis = KbmAxis_LeftBracket; break;
	case SDLK_RIGHTBRACKET:
		axis = KbmAxis_RightBracket; break;
	case SDLK_BACKSLASH:
		axis = KbmAxis_Backslash; break;
	case SDLK_SEMICOLON:
		axis = KbmAxis_Semicolon; break;
	case SDLK_QUOTE:
		axis = KbmAxis_Quote; break;
	case SDLK_RETURN:
		axis = KbmAxis_Return; break;
	case SDLK_COMMA:
		axis = KbmAxis_Comma; break;
	case SDLK_PERIOD:
		axis = KbmAxis_Period; break;
	case SDLK_SLASH:
		axis = KbmAxis_Slash; break;
	case SDLK_SPACE:
		axis = KbmAxis_Space; break;
	case SDLK_INSERT:
		axis = KbmAxis_Insert; break;
	case SDLK_DELETE:
		axis = KbmAxis_Delete; break;
	case SDLK_HOME:
		axis = KbmAxis_Home; break;
	case SDLK_END:
		axis = KbmAxis_End; break;
	case SDLK_PAGEUP:
		axis = KbmAxis_PageUp; break;
	case SDLK_PAGEDOWN:
		axis = KbmAxis_PageDown; break;
	case SDLK_LEFT:
		axis = KbmAxis_LeftArrow; break;
	case SDLK_RIGHT:
		axis = KbmAxis_RightArrow; break;
	case SDLK_UP:
		axis = KbmAxis_UpArrow; break;
	case SDLK_DOWN:
		axis = KbmAxis_DownArrow; break;
	case SDLK_KP_0:
		axis = KbmAxis_Pad0; break;
	case SDLK_KP_1:
		axis = KbmAxis_Pad1; break;
	case SDLK_KP_2:
		axis = KbmAxis_Pad2; break;
	case SDLK_KP_3:
		axis = KbmAxis_Pad3; break;
	case SDLK_KP_4:
		axis = KbmAxis_Pad4; break;
	case SDLK_KP_5:
		axis = KbmAxis_Pad5; break;
	case SDLK_KP_6:
		axis = KbmAxis_Pad6; break;
	case SDLK_KP_7:
		axis = KbmAxis_Pad7; break;
	case SDLK_KP_8:
		axis = KbmAxis_Pad8; break;
	case SDLK_KP_9:
		axis = KbmAxis_Pad9; break;
	case SDLK_NUMLOCKCLEAR:
		axis = KbmAxis_Numlock; break;
	case SDLK_KP_DIVIDE:
		axis = KbmAxis_PadDivide; break;
	case SDLK_KP_MULTIPLY:
		axis = KbmAxis_PadMultiply; break;
	case SDLK_KP_MINUS:
		axis = KbmAxis_PadMinus; break;
	case SDLK_KP_PLUS:
		axis = KbmAxis_PadPlus; break;
	case SDLK_KP_ENTER:
		axis = KbmAxis_PadEnter; break;
	case SDLK_KP_PERIOD:
		axis = KbmAxis_PadPeriod; break;
	case SDLK_CLEAR:
		axis = KbmAxis_Clear; break;
	case SDLK_KP_EQUALS:
		axis = KbmAxis_PadEquals; break;
	case SDLK_F13:
		axis = KbmAxis_F13; break;
	case SDLK_F14:
		axis = KbmAxis_F14; break;
	case SDLK_F15:
		axis = KbmAxis_F15; break;
	default:
		axis = KbmAxis_None; break;
	}
	return axis;
}

KbmAxis axisFromSdl(Uint8 mouseButton)
{
	KbmAxis axis;
	switch(mouseButton) {
	case SDL_BUTTON_LEFT:
		axis = KbmAxis_MouseLeft; break;
	case SDL_BUTTON_RIGHT:
		axis = KbmAxis_MouseRight; break;
	case SDL_BUTTON_MIDDLE:
		axis = KbmAxis_MouseMiddle; break;
	case SDL_BUTTON_X1:
		axis = KbmAxis_MouseButton4; break;
	case SDL_BUTTON_X2:
		axis = KbmAxis_MouseButton5; break;
	}
	return axis;
}
