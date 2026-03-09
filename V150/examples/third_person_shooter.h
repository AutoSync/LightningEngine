/*
* SPACE INVADERS CLONE IN LIGHTNING ENGINE
*
*/

#pragma once
#include "../src/include/Window.h"
#include "../src/include/Level.h"
#include "../src/include/Types.h"

using namespace Lightning;
using namespace LightningEngine;

class Player {
public:
	int health = 100;
	int power = 10;
	int speed = 5;
	int level = 1;
	V2 position = V2(0.0f, 0.0f);
};

class Level1 : public Level {
private:
	Player* player;
	Player* enemy;
	public:
	Level1(const char* name) {
		// Initialize level
		std::cout << "Level Created: " << name << std::endl;
	}
	void setPlayers(Player* p, Player* e) {
		player = p;
		enemy = e;
	}
	void Initialize() override {
		// Load level resources
	}
	void Shutdown() override {
		// Unload level resources
	}
	void Update(float dt) override {
		
		auto keyboardState = SDL_GetKeyboardState(NULL);
		if (keyboardState[SDL_SCANCODE_D]) player->position.x -= player->speed * dt;
		if (keyboardState[SDL_SCANCODE_A]) player->position.x += player->speed * dt;
		if (keyboardState[SDL_SCANCODE_W]) player->position.y -= player->speed * dt;
		if (keyboardState[SDL_SCANCODE_S]) player->position.y += player->speed * dt;
		if (keyboardState[SDL_SCANCODE_SPACE]) {
			// Fire bullet
			std::cout << "Player shoots!" << std::endl;
		}

		if (keyboardState[SDL_SCANCODE_ESCAPE]) {
			// Exit game
			std::cout << "Exit Game!" << std::endl;
		}

	}
	void Render() override {
		



	}
};

class SpaceIvaders : public LightningEngine::Window {
private: 
	Player player;
	Player enemy;
	Level1 level1 = Level1("Level 1 - blocks");

public:
	SpaceIvaders() : Window("Space Invaders - Lightning Engine", 800, 600, false) {
		// Game Title 
		title = "Space Invaders - Lightning Engine";
		width = 800;
		height = 600;

		Initialize();
	}
	void Initialize() override {
		//Start Game

		// set initial positions
		player.position = V2(400.0f, 500.0f);
		enemy.position = V2(400.0f, 100.0f);

		// set players in level
		level1.setPlayers(&player, &enemy);

		level1.Initialize();
	}
};