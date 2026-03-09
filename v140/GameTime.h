#pragma once
namespace LightningEngine
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

