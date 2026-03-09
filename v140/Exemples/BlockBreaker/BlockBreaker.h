#pragma once
#include "../../Engine.h"
#include "../../Shapes.h"
#include <memory>
#include <vector>

namespace LightningEngine
{
	struct Brick
	{
		V2 position;
		V2 size;
		LinearColor color;
		bool destroyed = false;
		int hitPoints;


		Brick(float x, float y, float width, float height, LinearColor c, int hp)
		{
			position = V2(x, y);
			size = V2(width, height);
			color = c;
			hitPoints = hp;
			destroyed = false;
		}


	};
	class Ball
	{
	public:
		V2 position;
		V2 velocity;
		float radius;
		LinearColor color;

		Ball(float x, float y, float r, LinearColor c, float velX, float velY) : position(x, y), radius(r), color(c)
		{
			velocity = V2(0.3f, 0.3f);
		}

		void Update(float deltaTime)
		{
			position.x += velocity.x * deltaTime;
			position.y += velocity.y * deltaTime;
		}

		void Reset(float x, float y)
		{
			position = V2(x, y);
			velocity = V2(0.3f, 0.3f);
		}
	};
	class Paddle {
		V2 position;
		V2 size;
		LinearColor color;
		float speed;

		Paddle(float x, float y, float width, float height, LinearColor c, float s)
			: position(x, y), size(width, height), color(c), speed(s)
		{
			/* Empty */
		}

		void Update(float deltaTime, int windowWidth)
		{
			position.x += speed * deltaTime * 60;

			if (position.x < 0)
			{
				position.x = 0;
			}
			else if (position.x + size.x > windowWidth)
			{
				position.x = windowWidth - size.x;
			}

		}

		void MoveLeft()
		{
			speed = -0.5f;
		}
		void MoveRight()
		{
			speed = 0.5f;
		}
		void Stop()
		{
			speed = 0.0f;
		}

	};

	class Level
	{
	private:
		std::vector<std::vector<int>> layout;
		float brickWidth, brickHeight;
		float spacing;
	public:
		Level() : brickWidth(60.0f), brickHeight(20.0f), spacing(5.0f)
		{
			/* Empty */
		}
		void LoadLevel(int levelNumber)
		{
			layout.clear();

			switch (levelNumber)
			{
			case 1:
				layout = {
					{1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
					{1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
					{1, 0, 2, 2, 2, 2, 2, 2, 0, 1},
					{1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
					{1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
				};
				break;
			case 2:
				layout = {
					{2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
					{2, 1, 1, 1, 1, 1, 1, 1, 1, 2},
					{2, 1, 0, 0, 0, 0, 0, 0, 1, 2},
					{2, 1, 0, 3, 3, 3, 3, 0, 1, 2},
					{2, 1, 0, 0, 0, 0, 0, 0, 1, 2},
					{2, 1, 1, 1, 1, 1, 1, 1, 1, 2},
					{2, 2, 2, 2, 2, 2, 2, 2, 2, 2}
				};
				break;
			case 3:
				layout = {
					{3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
					{3, 2, 2, 2, 2, 2, 2, 2, 2, 3},
					{3, 2, 1, 1, 1, 1, 1, 1, 2, 3},
					{3, 2, 1, 0, 0, 0, 0, 1, 2, 3},
					{3, 2, 1, 0, 4, 4, 0, 1, 2, 3},
					{3, 2, 1, 0, 0, 0, 0, 1, 2, 3},
					{3, 2, 1, 1, 1, 1, 1, 1, 2, 3},
					{3, 2, 2, 2, 2, 2, 2, 2, 2, 3},
					{3, 3, 3, 3, 3, 3, 3, 3, 3, 3}
				};
				break;
			default:
				layout = {
					{1, 1, 1, 1},
					{1, 1, 1, 1},
					{1, 1, 1, 1}
				};
			}
		}
		std::vector<Brick> GenerateBricks(float startX, float startY)
		{
			std::vector<Brick> bricks;
			for(int row = 0; row < layout.size(); ++row)
			{
				for(int col = 0; col < layout[row].size(); ++col)
				{
					int brickType = layout[row][col];
					if(brickType > 0)
					{
						float x = startX + col * (brickWidth + spacing);
						float y = startY + row * (brickHeight + spacing);
						LinearColor color;
						int hitPoints;
						switch(brickType)
						{
						case 1:
							color = LinearColor(1.0f, 0.0f, 0.0f, 1.0f); // Red
							hitPoints = 1;
							break;
						case 2:
							color = LinearColor(0.0f, 1.0f, 0.0f, 1.0f); // Green
							hitPoints = 2;
							break;
						case 3:
							color = LinearColor(0.0f, 0.0f, 1.0f, 1.0f); // Blue
							hitPoints = 3;
							break;
						case 4:
							color = LinearColor(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
							hitPoints = 4;
							break;
						default:
							color = LinearColor(1.0f, 1.0f, 1.0f, 1.0f); // White
							hitPoints = 1;
						}
						bricks.emplace_back(x, y, brickWidth, brickHeight, color, hitPoints);
					}
				}
			}
		}
	};
	class BlockBreakerGame : public EngineCore 
	{
	private:
		std::unique_ptr<Shapes> shapeRenderer;
		std::unique_ptr<Paddle> paddle;
		std::unique_ptr<Ball> ball;
		std::vector<Brick> bricks;
		std::unique_ptr<Level> level;

		int score;
		int lives;
		int currentLevel;
		bool gameStarted;
	public:
		BlockBreakerGame(EngineSettings setts);
		void Run() override;
		void Start() override;
		void End() override;
		void Update() override;
		void Render() override;
	private:
		void processInput();
		void checkCollisions();
		void ResetGame();
		void LoadNextLevel();
		bool checkLevelComplete();
		void DrawUI();
	};
}