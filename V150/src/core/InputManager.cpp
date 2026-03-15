#include "../include/InputManager.h"

namespace LightningEngine {

	void InputManager::Update()
	{
		previousKeys     = currentKeys;
		for (int i = 0; i < 5; ++i)
			prevMouseButtons[i] = mouseButtons[i];
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

}
