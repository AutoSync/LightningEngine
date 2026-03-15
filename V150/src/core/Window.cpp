#include "../include/Window.h"
#include <iostream>

namespace LightningEngine {

	bool Window::onInit(const char* title, int width, int height, bool fullscreen)
	{
		if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS)) {
			std::cerr << "[Window] SDL_Init failed: " << SDL_GetError() << "\n";
			return false;
		}

		Uint32 flags = SDL_WINDOW_RESIZABLE;
		if (fullscreen) flags |= SDL_WINDOW_FULLSCREEN;

		sdlWindow = SDL_CreateWindow(title, width, height, flags);
		if (!sdlWindow) {
			std::cerr << "[Window] SDL_CreateWindow failed: " << SDL_GetError() << "\n";
			SDL_Quit();
			return false;
		}

		// Create GPU device — requests SPIRV (Vulkan backend).
		// For multi-backend support, pass SDL_GPU_SHADERFORMAT_SPIRV | DXBC | DXIL.
		gpuDevice = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, nullptr);
		if (!gpuDevice) {
			std::cerr << "[Window] SDL_CreateGPUDevice failed: " << SDL_GetError() << "\n";
			SDL_DestroyWindow(sdlWindow);
			sdlWindow = nullptr;
			SDL_Quit();
			return false;
		}

		if (!SDL_ClaimWindowForGPUDevice(gpuDevice, sdlWindow)) {
			std::cerr << "[Window] SDL_ClaimWindowForGPUDevice failed: " << SDL_GetError() << "\n";
			SDL_DestroyGPUDevice(gpuDevice);
			gpuDevice = nullptr;
			SDL_DestroyWindow(sdlWindow);
			sdlWindow = nullptr;
			SDL_Quit();
			return false;
		}

		this->width  = width;
		this->height = height;

		renderer = Renderer(gpuDevice, sdlWindow);
		return true;
	}

	void Window::onShutdown()
	{
		// Release Renderer GPU resources before destroying the device.
		renderer.Release();

		if (gpuDevice) {
			SDL_ReleaseWindowFromGPUDevice(gpuDevice, sdlWindow);
			SDL_DestroyGPUDevice(gpuDevice);
			gpuDevice = nullptr;
		}
		if (sdlWindow) {
			SDL_DestroyWindow(sdlWindow);
			sdlWindow = nullptr;
		}
		SDL_Quit();
	}

	void Window::onHandleEvents()
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
			inputManager.ProcessEvent(event);

		if (inputManager.ShouldQuit())
			isRunning = false;
	}

	void Window::Run(const char* title, int width, int height, bool fullscreen)
	{
		if (!onInit(title, width, height, fullscreen))
			return;

		Initialize();
		isRunning = true;

		Uint64 prevTime = SDL_GetPerformanceCounter();

		while (isRunning) {
			inputManager.Update();   // snapshot previous frame state
			onHandleEvents();        // fill current frame state

			Uint64 now  = SDL_GetPerformanceCounter();
			float  dt   = (float)(now - prevTime) * 1000.f / (float)SDL_GetPerformanceFrequency();
			prevTime    = now;

			Update(dt);
			Render();
		}

		Shutdown();
		onShutdown();
	}

}
