// Description: Header file for window management in a graphical application.
// -------------------------- MAP OF THE CLASSES GAME ------------------------
// ------ [ WINDOW ] - GAMEINSTANCE - LEVEL - NODES - COMPONENTS -------------
// --------------------------------------------------------------------------- 

#pragma once
#include <SDL3/SDL.h>
#include <memory>
#include <iostream>


namespace LightningEngine {
	class Window {
	public:
		const char* title = "Lightning Engine";
		const char* rendererName = "untitled game";
		int width, height;
		bool isFullscreen;
		bool isRunning = true;
		SDL_Window* sdlWindow;
		SDL_Renderer* sdlRenderer;
	public:
		Window(const char* title, int width, int height, bool is_fullscreen);
		void Run(const char* title, int width, int height, bool is_fullscreen);
		virtual void Initialize() = 0;
		virtual void Shutdown() = 0;
		virtual void Update(float dt) = 0;
		virtual void Render() = 0;
		SDL_Renderer* getSDLRenderer() const { return sdlRenderer; }
	private:
		void onInit();
		void onShutdown();
		void onUpdate(float dt);
		void onRender();
		void onHandleEvents();
	};
}