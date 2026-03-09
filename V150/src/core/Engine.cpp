#include "../include/Engine.h"
#include "../include/GameInstance.h"
#include "../include/Renderer.h"
#include "../Include/InputManager.h"
#include <iostream>

namespace LightningEngine {

	
	Engine::Engine() : isRunning(false), window(nullptr), previewsTime(0.0f), deltaTime(0.0f){}
	Engine::~Engine() { Shutdown(); }

	bool Engine::Initialize(const std::string& title, int width, int height)
	{
		if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS))
		{
			std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
			return false;
		}

		previewsTime = SDL_GetPerformanceCounter();
		return true;
	}

	void Engine::Shutdown()
	{
		if (this->gameNode)
		{
			gameNode->Shutdown();
		}

		if (window)
		{
			SDL_DestroyWindow(window);
			window = nullptr;
		}

		SDL_Quit();
	}

	void Engine::HandleEvents()
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

	void Engine::Update()
	{
		if (gameNode)
		{
			gameNode->Update(deltaTime);
		}
	}

	void Engine::Render()
	{
		if (gameNode)
		{
			gameNode->Render();
		}
	}

	void Engine::Run(std::unique_ptr<LightningEngine::GameInstance> gameInstance, const std::string& title, int width, int height)
	{
		if(!Initialize(title, width, height))
		{
			return;
		}

		gameNode = std::move(gameInstance);
		gameNode->Initialize();

		isRunning = true;
		
		while(isRunning)
		{
			HandleEvents();
			Update();

			Uint64 currentTime = SDL_GetPerformanceCounter();
			deltaTime = (float)(currentTime - previewsTime) * 1000 / (double)SDL_GetPerformanceFrequency();
			previewsTime = currentTime;
			Render();
			SDL_GL_SwapWindow(window);
		}

		Shutdown();
	}



}
