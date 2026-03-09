/*2018 - 2025 (C) AutoSync Lightning Engine - By Erick Andrade All Rights Reserved
*/

#pragma once
#include "src/include/Engine.h"
#include "src/include/GameInstance.h"
#include "src/include/Renderer.h"
#include <SDL3/SDL.h>


// GAMES
#include "examples/third_person_shooter.h"
#include <iostream>

class MyGame : public LightningEngine::GameInstance
{
private:
	std::unique_ptr<LightningEngine::Renderer> renderer;
	SDL_Window* window;
	float playerx, playery;
public:

	void Initialize() override
	{
		title = "My Game";
		width = 800;
		height = 600;

		window = SDL_CreateWindow(title, width, height, SDL_WINDOW_RESIZABLE);


		renderer = std::make_unique<LightningEngine::Renderer>(window);

		playerx = 400.f;
		playery = 300.f;

		std::cout << "Game Started" << std::endl;
	}

	void Shutdown() override
	{
		std::cout << "Game Stopped" << std::endl;
	}

	void Update(float dt) override
	{
		auto keyboardState = SDL_GetKeyboardState(NULL);

		if (keyboardState[SDL_SCANCODE_W]) playery -= 10.f * dt;
		if (keyboardState[SDL_SCANCODE_S]) playery += 10.f * dt;
		if (keyboardState[SDL_SCANCODE_A]) playerx -= 10.f * dt;
		if (keyboardState[SDL_SCANCODE_D]) playerx += 10.f * dt;

		if (keyboardState[SDL_SCANCODE_ESCAPE]) {

		}

	}

	void Render() override
	{
		if (!renderer) return;

		renderer->Clear();

		renderer->SetDrawColor(255, 0, 0, 255);
		renderer->FillRect(playerx - 25, playery - 25, 50, 50);

		renderer->SetDrawColor(0, 255, 0, 255);
		renderer->FillRect(100, 100, 30, 30);

		renderer->SetDrawColor(0, 0, 255, 255);
		renderer->FillRect(600, 400, 40, 40);

		renderer->Present();

	}

};

int main(int argc, char* argv[]) {

	LightningEngine::Engine engine;

	auto game = std::make_unique<MyGame>();
	engine.Run(std::move(game), "Lightning Engine Demo", 800, 600);

	return 0;
}