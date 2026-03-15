#pragma once
#include <SDL3/SDL.h>
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

		bool ShouldQuit() const { return quitRequested; }
	};

}

