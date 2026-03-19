#include "../include/InputManager.h"

namespace LightningEngine {

	void InputManager::Update()
	{
		previousKeys     = currentKeys;
		for (int i = 0; i < 5; ++i)
			prevMouseButtons[i] = mouseButtons[i];
		textBuf.clear();
		deletedBack  = false;
		scrollWheelY = 0.f;

		// Gamepad: open first available if none connected
		if (!gamepad) {
			int count = 0;
			SDL_JoystickID* ids = SDL_GetGamepads(&count);
			if (ids && count > 0)
				gamepad = SDL_OpenGamepad(ids[0]);
			SDL_free(ids);
		}

		// Snapshot previous gamepad button state, then read current
		for (int i = 0; i < kMaxButtons; ++i)
			prevGamepadButtons[i] = gamepadButtons[i];

		if (gamepad) {
			for (int i = 0; i < kMaxButtons; ++i)
				gamepadButtons[i] = SDL_GetGamepadButton(gamepad, (SDL_GamepadButton)i);
			for (int i = 0; i < SDL_GAMEPAD_AXIS_COUNT; ++i) {
				Sint16 raw = SDL_GetGamepadAxis(gamepad, (SDL_GamepadAxis)i);
				gamepadAxes[i] = raw / 32767.f;
			}
		}
	}

	void InputManager::ProcessEvent(const SDL_Event& event)
	{
		switch (event.type) {
		case SDL_EVENT_QUIT:
			quitRequested = true;
			break;

		case SDL_EVENT_KEY_DOWN:
			if (!event.key.repeat)
				currentKeys[event.key.scancode] = true;
			if (event.key.scancode == SDL_SCANCODE_BACKSPACE)
				deletedBack = true;
			break;

		case SDL_EVENT_KEY_UP:
			currentKeys[event.key.scancode] = false;
			break;

		case SDL_EVENT_MOUSE_MOTION:
			mouseX = event.motion.x;
			mouseY = event.motion.y;
			break;

		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			if (event.button.button >= 1 && event.button.button <= 5)
				mouseButtons[event.button.button - 1] = true;
			break;

		case SDL_EVENT_MOUSE_BUTTON_UP:
			if (event.button.button >= 1 && event.button.button <= 5)
				mouseButtons[event.button.button - 1] = false;
			break;

		case SDL_EVENT_TEXT_INPUT:
			textBuf += event.text.text;
			break;

		case SDL_EVENT_MOUSE_WHEEL:
			scrollWheelY += event.wheel.y;
			break;

		case SDL_EVENT_GAMEPAD_REMOVED:
			if (gamepad && SDL_GetGamepadID(gamepad) == event.gdevice.which) {
				SDL_CloseGamepad(gamepad);
				gamepad = nullptr;
				for (int i = 0; i < kMaxButtons; ++i)
					gamepadButtons[i] = prevGamepadButtons[i] = false;
				for (int i = 0; i < SDL_GAMEPAD_AXIS_COUNT; ++i)
					gamepadAxes[i] = 0.f;
			}
			break;
		}
	}

	// --- Keyboard ---

	bool InputManager::IsKeyDown(SDL_Scancode key) const
	{
		auto it = currentKeys.find(key);
		return it != currentKeys.end() && it->second;
	}

	bool InputManager::IsKeyPressed(SDL_Scancode key) const
	{
		auto it  = currentKeys.find(key);
		auto it2 = previousKeys.find(key);
		bool curr = it  != currentKeys.end()  && it->second;
		bool prev = it2 != previousKeys.end() && it2->second;
		return curr && !prev;
	}

	bool InputManager::IsKeyReleased(SDL_Scancode key) const
	{
		auto it  = currentKeys.find(key);
		auto it2 = previousKeys.find(key);
		bool curr = it  != currentKeys.end()  && it->second;
		bool prev = it2 != previousKeys.end() && it2->second;
		return !curr && prev;
	}

	// --- Mouse ---

	bool InputManager::IsMouseDown(int button) const
	{
		if (button < 1 || button > 5) return false;
		return mouseButtons[button - 1];
	}

	bool InputManager::IsMousePressed(int button) const
	{
		if (button < 1 || button > 5) return false;
		return mouseButtons[button - 1] && !prevMouseButtons[button - 1];
	}

	bool InputManager::IsMouseReleased(int button) const
	{
		if (button < 1 || button > 5) return false;
		return !mouseButtons[button - 1] && prevMouseButtons[button - 1];
	}

	// --- Gamepad ---

	bool InputManager::IsGamepadButtonDown(int btn) const
	{
		if (btn < 0 || btn >= kMaxButtons) return false;
		return gamepadButtons[btn];
	}

	bool InputManager::IsGamepadButtonPressed(int btn) const
	{
		if (btn < 0 || btn >= kMaxButtons) return false;
		return gamepadButtons[btn] && !prevGamepadButtons[btn];
	}

	bool InputManager::IsGamepadButtonReleased(int btn) const
	{
		if (btn < 0 || btn >= kMaxButtons) return false;
		return !gamepadButtons[btn] && prevGamepadButtons[btn];
	}

	float InputManager::GetGamepadAxis(SDL_GamepadAxis axis) const
	{
		int i = (int)axis;
		if (i < 0 || i >= SDL_GAMEPAD_AXIS_COUNT) return 0.f;
		return gamepadAxes[i];
	}

}
