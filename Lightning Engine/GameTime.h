#pragma once
namespace Lightning
{
	class GameTime
	{
	public:
		GameTime();
		void SetDeltaTime(float deltaTime);
		float deltaTime;
	private:
		float currentTime, lastFrame;
	};
}

