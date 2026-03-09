/* SPACE IVADERS CLONE IN LIGHTNING ENGINE
*/

#pragma once
#include "../../Engine.h"			// Engine Core for window and context
#include "../../Shapes.h"			// Shapes for rendering
#include "../../Types.h"
#include <vector>

using namespace LightningEngine;

//GameObject
struct SIObject {
	V2 position;
	V2 size;
	bool active;
	
};


class SpaceIvaders : public EngineCore
{
private:

	//Game
	SIObject player;
	std::vector<SIObject> bullets;
	std::vector<SIObject> invaders;
	std::vector<SIObject> barriers;
	float invaderDirection = 1.0f;
	double lastInvaderMoveTime = 0.0f;
	double invaderMoveInterval = 0.5f;

	// Game Settins
	float speed = 300.0f;
	float bulletSpeed = 400.0f;
	float invaderSpeed = 50.0f;

	// Window size
	int width = 1280;
	int height = 720;

	// For render
	Shapes shape;

public:
	SpaceIvaders() 
	{
		settings.title = "Space Invaders - Lightning Engine";
		settings.width = width;
		settings.height = height;

		InitializeWindow(settings);
	}

	void Run() override
	{
		// Engine Initialization
		InitializeEngine();
	}
	void Start() override
	{
		// Initialize player
		player.position.x = width / 2.0f - 25.0f;
		player.position.y = 50.0f;
		player.size.x = 50.0f;
		player.size.y = 20.0f;
		player.active = true;

		// Initialize Invaders
		invaders.clear();
		for (int row = 0; row < 5; row++) {
			for (int col = 0; col < 10; col++) {
				invaders.push_back({
				{ 80.0f + col * 60.0f,
				height - 100.0f - row * 40.0f },
				{ 40.0f, 30.0f },
				true });
			}
		}

		// Initialize Barriers
		barriers.clear();
		for (int i = 0; i < 4; i++) {
			barriers.push_back({
			{ 150.0f + i * 200.0f,
			120.0f },
			{ 80.0f, 40.0f },
			true });
		}

		bullets.clear();
	}

	void Update() override
	{
	
		double currentTime = Time->GetTime();
		// Invaders Movement
		if (currentTime - lastInvaderMoveTime > invaderMoveInterval)
		{
			for(auto& invader : invaders)
			{
				if (invader.active)
				{
					invader.position.x + invaderSpeed * invaderDirection;

					if (invader.position.x <= 0 || invader.position.x + invader.size.x >= width)
					{
						invaderDirection *= -1;
						for (auto& inv : invaders)
						{
							inv.position.y -= 20.0f;
						}
						break;
					}
				}
			}
		}
		lastInvaderMoveTime = currentTime;

		// Bullets
		for(auto& bullet : bullets)
		{
			if (bullet.active)
			{
				bullet.position.y += bulletSpeed * Time->deltaTime;

				// Check off screen
				if (bullet.position.y > height)
					bullet.active = false;

				// Check collision with invaders
				for (auto& invader : invaders)
				{
					if (invader.active && checkCollision(bullet, invader))
					{
						invader.active = false;
						bullet.active = false;
						break;
					}
				}

				// Check collision with barriers
				for (auto& barrier : barriers)
				{
					if (barrier.active && checkCollision(bullet, barrier))
					{
						barrier.active = false;
						bullet.active = false;
						break;
					}
				}
			}
		}

		// Remove inactive bullets
		bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
			[](const SIObject& obj) { return !obj.active; }),
			bullets.end());
	}


	void movePlayer(float direction)
	{
		player.position.x += direction * speed * Time->deltaTime;

		// Clamp within window bounds
		if(player.position.x < 0) 
			player.position.x = 0;
		if (player.position.x + player.size.x > width)
		{
			player.position.x = width - player.size.x;
		}
	}

	void shoot(float direction)
	{
		SIObject newBullet;
		newBullet.position.x = player.position.x + player.size.x / 2.0f - 2.5f;
		newBullet.position.y = player.position.y + player.size.y;
		newBullet.size.x = 5.0f;
		newBullet.size.y = 15.0f;
		newBullet.active = true;
		bullets.push_back(newBullet);
	}

	void Render() override
	{
		auto background = LinearColor(C3(43, 42, 39), 1.0f);
		SetClearColor(background, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Render Player
		shape.DrawRect(player.position, player.size, C3(0, 255, 0));

		// Render Invaders
		for (const auto& invader : invaders)
		{
			if (invader.active)
				shape.DrawRect(invader.position, invader.size, C3(255, 255, 0));
		}

		// Render Barriers
		for (const auto& barrier : barriers)
		{
			if (barrier.active)
				shape.DrawRect(barrier.position, barrier.size, C3(0, 0, 255));
		}
	}

	void End() override
	{

	}

	bool checkCollision(const SIObject& a, const SIObject& b)
	{
		return a.position.x < b.position.x + b.size.x &&
			a.position.x + a.size.x > b.position.x &&
			a.position.y < b.position.y + b.size.y &&
			a.position.y + a.size.y > b.position.y;
	}

	
};