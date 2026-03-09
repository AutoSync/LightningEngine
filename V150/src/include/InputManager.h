#pragma once
#include <SDL3/SDL.h>
#include <unordered_map>


namespace LightningEngine
{
	class InputManager
	{
	private:
		std::unordered_map<SDL_Keycode, bool> KeyStates;
		float MouseX, MouseY;
		bool quitRequested;
	public:
		InputManager();

		void ProcessEvent(const SDL_Event& event);
		void Update();

		bool IsKeyPressed(SDL_Keycode key) const;
		bool IsKeyDown(SDL_Keycode key) const;
		bool IsKeyReleased(SDL_Keycode key) const;

		float GetMouseX() const { return MouseX; }
		float GetMouseY() const { return MouseY; }
		bool ShouldQuit() const { return quitRequested; }
	};
}

