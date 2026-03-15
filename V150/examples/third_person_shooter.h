// Space Invaders — Example game built with Lightning Engine.
// Demonstrates: GameInstance, Level, and basic game objects.
//
// Usage:
//   SpaceInvaders game;
//   game.Run("Space Invaders", 800, 600);

#pragma once
#include "../src/include/GameInstance.h"
#include "../src/include/Renderer.h"
#include "../src/include/Level.h"
#include "../src/include/Types.h"

using namespace Lightning;
using namespace LightningEngine;

// -----------------------------------------------------------------------
// Game objects
// -----------------------------------------------------------------------
class Player {
public:
	int   health   = 100;
	int   power    = 10;
	float speed    = 5.f;
	int   level    = 1;
	V2    position = V2(0.f, 0.f);
};

// -----------------------------------------------------------------------
// Level1
// -----------------------------------------------------------------------
class Level1 : public Level {
private:
	Player* player = nullptr;
	Player* enemy  = nullptr;

public:
	Level1(const char* name)
	{
		std::cout << "[Level] Created: " << name << std::endl;
	}

	void setPlayers(Player* p, Player* e) { player = p; enemy = e; }

	void Initialize() override {}
	void Shutdown()   override {}

	void Update(float dt) override
	{
		if (!player || !input) return;
		if (input->IsKeyDown(SDL_SCANCODE_D)) player->position.x += player->speed * dt;
		if (input->IsKeyDown(SDL_SCANCODE_A)) player->position.x -= player->speed * dt;
		if (input->IsKeyDown(SDL_SCANCODE_W)) player->position.y -= player->speed * dt;
		if (input->IsKeyDown(SDL_SCANCODE_S)) player->position.y += player->speed * dt;
	}

	void Render() override {}
};

// -----------------------------------------------------------------------
// SpaceInvaders — extends GameInstance
// -----------------------------------------------------------------------
class SpaceInvaders : public GameInstance {
private:
	Player player;
	Player enemy;
	Level1 level1{ "Level 1 - Blocks" };

public:
	void Initialize() override
	{
		renderer.SetClearColor(20, 20, 30);

		player.position = V2(400.f, 500.f);
		enemy.position  = V2(400.f, 100.f);

		level1.SetContext(renderer, inputManager);
		level1.setPlayers(&player, &enemy);
		level1.Initialize();
	}

	void Shutdown() override
	{
		level1.Shutdown();
	}

	void Update(float dt) override
	{
		if (inputManager.IsKeyPressed(SDL_SCANCODE_ESCAPE)) Quit();
		level1.Update(dt);
	}

	void Render() override
	{
		renderer.Clear();

		// Player (red)
		renderer.SetDrawColor(220, 50, 50, 255);
		renderer.FillRect(player.position.x - 15.f, player.position.y - 15.f, 30.f, 30.f);

		// Enemy (yellow)
		renderer.SetDrawColor(220, 200, 50, 255);
		renderer.FillRect(enemy.position.x - 15.f, enemy.position.y - 15.f, 30.f, 30.f);

		renderer.Present();
	}
};
