// Window — Base class of the engine hierarchy.
// Owns SDL_Window + SDL_GPUDevice. Manages SDL lifecycle and the main loop.
// SDL/GPU internals are private — subclasses interact only through Renderer and InputManager.
//
// CLASS HIERARCHY:
// [ WINDOW ] -> GAMEINSTANCE -> LEVEL -> NODES -> COMPONENTS

#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include "Renderer.h"
#include "InputManager.h"

namespace LightningEngine {

	class Window {
	protected:
		Renderer     renderer;
		InputManager inputManager;
		bool         isRunning = false;

	public:
		virtual ~Window() = default;

		// Starts the engine: initializes SDL + GPU device, runs loop.
		void Run(const char* title, int width = 1280, int height = 720, bool fullscreen = false);

		// Signals the engine to stop at end of current frame.
		void Quit() { isRunning = false; }

		int GetWidth()  const { return width; }
		int GetHeight() const { return height; }

		// --- Game lifecycle (must be implemented by subclass) ---
		virtual void Initialize()            = 0;
		virtual void Shutdown()              = 0;
		virtual void Update(float deltaTime) = 0;
		virtual void Render()                = 0;

	private:
		SDL_Window*    sdlWindow  = nullptr;
		SDL_GPUDevice* gpuDevice  = nullptr;
		int            width      = 1280;
		int            height     = 720;

		bool onInit(const char* title, int width, int height, bool fullscreen);
		void onShutdown();
		void onHandleEvents();
	};

}
