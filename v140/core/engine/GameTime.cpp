#include "../../GameTime.h"

LightningEngine::GameTime::GameTime()
{
	time = 0;
	deltaTime = 0;
	currentTime = 0;
	lastFrame = 0;
}

void LightningEngine::GameTime::SetDeltaTime(double dt)
{
	time = dt;
	currentTime = time;
	deltaTime = currentTime - lastFrame;
	lastFrame = currentTime;
}

double LightningEngine::GameTime::GetTime()
{
	return time;
}

void LightningEngine::GameTime::SetTime(double t)
{
	this->time = t;
}