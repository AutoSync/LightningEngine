#pragma once
#include <SDL3/SDL.h>
#include <string>
#include <unordered_map>

namespace LightningEngine {

	class InputManager {
	private:
		std::unordered_map<SDL_Scancode, bool> currentKeys;
		std::unordered_map<SDL_Scancode, bool> previousKeys;

		bool  mouseButtons[5]     = {};
		bool  prevMouseButtons[5] = {};
		float mouseX = 0.f, mouseY = 0.f;

		bool quitRequested = false;
		std::string textBuf;
		bool        deletedBack  = false;
		float       scrollWheelY = 0.f;

		// Gamepad
		SDL_Gamepad* gamepad = nullptr;
		static constexpr int kMaxButtons = 21;  // SDL_GAMEPAD_BUTTON_COUNT
		bool  gamepadButtons[kMaxButtons]     = {};
		bool  prevGamepadButtons[kMaxButtons] = {};
		float gamepadAxes[SDL_GAMEPAD_AXIS_COUNT] = {};

	public:
		// Called once per frame before ProcessEvent — snapshots previous state.
		void Update();

		// Called for each SDL_Event polled from the queue.
		void ProcessEvent(const SDL_Event& event);

		// --- Keyboard ---
		bool IsKeyDown(SDL_Scancode key)     const;  // held this frame
		bool IsKeyPressed(SDL_Scancode key)  const;  // went down this frame
		bool IsKeyReleased(SDL_Scancode key) const;  // went up this frame

		// --- Mouse ---
		bool  IsMouseDown(int button)     const;  // button: 1=left 2=middle 3=right
		bool  IsMousePressed(int button)  const;
		bool  IsMouseReleased(int button) const;
		float GetMouseX() const { return mouseX; }
		float GetMouseY() const { return mouseY; }

		bool  ShouldQuit()      const { return quitRequested; }
		const std::string& GetTextInput() const { return textBuf; }
		bool  HasDeleteBack()   const { return deletedBack; }
		float GetScrollWheelY() const { return scrollWheelY; }

		// --- Gamepad ---
		bool  HasGamepad() const { return gamepad != nullptr; }

		// btn: SDL_GamepadButton values (e.g. SDL_GAMEPAD_BUTTON_SOUTH)
		bool  IsGamepadButtonDown(int btn)     const;
		bool  IsGamepadButtonPressed(int btn)  const;
		bool  IsGamepadButtonReleased(int btn) const;

		// axis: SDL_GamepadAxis values (e.g. SDL_GAMEPAD_AXIS_LEFTX)
		// Returns normalised value in [-1, 1].
		float GetGamepadAxis(SDL_GamepadAxis axis) const;
	};

}

