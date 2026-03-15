// Lightning Engine — Editor / Entry Point
// 2018-2025 (C) AutoSync Lightning Engine - By Erick Andrade. All Rights Reserved.
//
// Extend GameInstance to create your game.
// Call game.Run("Title", width, height) in main() to start.

#include "src/include/GameInstance.h"
#include "src/include/Types.h"

// --- Example game scenes ---
#include "examples/third_person_shooter.h"
#include "examples/rpg_combat.h"

// -----------------------------------------------------------------------
// MyGame — minimal sandbox for testing the engine during development.
// -----------------------------------------------------------------------
class MyGame : public LightningEngine::GameInstance {
private:
	float playerx = 400.f;
	float playery = 300.f;

public:
	void Initialize() override
	{
		renderer.SetClearColor(30, 30, 30);
	}

	void Shutdown() override {}

	void Update(float dt) override
	{
		if (inputManager.IsKeyDown(SDL_SCANCODE_W)) playery -= 0.3f * dt;
		if (inputManager.IsKeyDown(SDL_SCANCODE_S)) playery += 0.3f * dt;
		if (inputManager.IsKeyDown(SDL_SCANCODE_A)) playerx -= 0.3f * dt;
		if (inputManager.IsKeyDown(SDL_SCANCODE_D)) playerx += 0.3f * dt;

		if (inputManager.IsKeyPressed(SDL_SCANCODE_ESCAPE)) Quit();
	}

	void Render() override
	{
		renderer.Clear();

		renderer.SetDrawColor(255, 0, 0);
		renderer.FillRect(playerx - 25.f, playery - 25.f, 50.f, 50.f);

		renderer.SetDrawColor(0, 255, 0);
		renderer.FillRect(100.f, 100.f, 30.f, 30.f);

		renderer.SetDrawColor(0, 0, 255);
		renderer.FillRect(600.f, 400.f, 40.f, 40.f);

		renderer.Present();
	}
};

// -----------------------------------------------------------------------
int main(int argc, char* argv[])
{
	RPGGame game;
	game.Run("Lightning Engine — RPG Combat 2D", 800, 600);
	return 0;
}
