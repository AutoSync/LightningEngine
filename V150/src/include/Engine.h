/* 2018-2025 (C) AutoSync Lightning Engine by Erick Andrade
* All rights reserved
* This file is part of Lightning Engine
* 
*/

#pragma once
#include <SDL3/SDL.h>
#include <string>
#include <memory>
#include "GameInstance.h"

namespace LightningEngine 
{
	class Engine
	{
	private:
		bool isRunning;
		SDL_Window* window;
		std::unique_ptr<GameInstance> gameNode;
		Uint64 previewsTime;
		float deltaTime;

		bool Initialize(const std::string& title, int width, int height);
		void Shutdown();
		void Update();
		void HandleEvents();
		void Render();
	public:
		Engine();
		~Engine();

		void Run(std::unique_ptr<GameInstance> gameInstance, const std::string& title = "Lightning Engine", int width = 1280, int height = 720);
		float getDeltaTime() const { return deltaTime; }

	};
}