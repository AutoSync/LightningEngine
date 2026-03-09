#include "../include/Window.h"

namespace LightningEngine 
{
		Window::Window(const char* title, int width, int height, bool is_fullscreen)
		: title(title), width(width), height(height), isFullscreen(is_fullscreen), sdlWindow(nullptr), sdlRenderer(nullptr)
	{
		onInit();
	}

		void Window::Run(const char* title, int width, int height, bool is_fullscreen)
		{

		}

	void Window::onInit()
	{
		Uint32 flags = SDL_WINDOW_RESIZABLE;
		if (isFullscreen) {
			flags |= SDL_WINDOW_FULLSCREEN;
		}
		sdlWindow = SDL_CreateWindow(title, width, height, flags);
		if (!sdlWindow) {
			std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
			return;
		}
		sdlRenderer = SDL_CreateRenderer(sdlWindow, "lightning engine");
		if (!sdlRenderer) {
			std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
			SDL_DestroyWindow(sdlWindow);
			sdlWindow = nullptr;
			return;
		}
	}
	void Window::onShutdown()
	{
		if (sdlRenderer) {
			SDL_DestroyRenderer(sdlRenderer);
			sdlRenderer = nullptr;
		}
		if (sdlWindow) {
			SDL_DestroyWindow(sdlWindow);
			sdlWindow = nullptr;
		}
	}
	void Window::onUpdate(float dt)
	{
		this->Update(dt);
	}
	void Window::onRender()
	{
		this->Render();
	}
	void Window::onHandleEvents()
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_EVENT_QUIT)
			{
				isRunning = false;
			}
		}
	}
}