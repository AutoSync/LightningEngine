#pragma once
namespace Lightning
{
	class GameTime
	{
	public:
		double deltaTime, currentTime, lastFrame, time;
	public:
		GameTime();
		void SetDeltaTime(double dt);
		double GetTime();
		void SetTime(double t);
	};
}

