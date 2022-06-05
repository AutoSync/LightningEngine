#include "GameTime.h"

Lightning::GameTime::GameTime()
{
	time = 0;
	deltaTime = 0;
	currentTime = 0;
	lastFrame = 0;
}

void Lightning::GameTime::SetDeltaTime(double dt)
{
	time = dt;
	currentTime = time;
	deltaTime = currentTime - lastFrame;
	lastFrame = currentTime;
}

double Lightning::GameTime::GetTime()
{
	return time;
}

void Lightning::GameTime::SetTime(double t)
{
	this->time = t;
}