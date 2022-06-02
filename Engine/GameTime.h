#pragma once
namespace Lightning
{
	class GameTime
	{
	public:
		static double deltaTime, currentTime, lastFrame, time;
	public:
		GameTime();
		void SetDeltaTime(double deltaTime);
		double GetTime();
		void SetTime(double t);
	};
}

