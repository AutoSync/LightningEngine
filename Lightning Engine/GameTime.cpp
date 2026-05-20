#include "GameTime.h"

Lightning::GameTime::GameTime()
{
	deltaTime = 0;
	currentTime = 0;
	lastFrame = 0;
}

void Lightning::GameTime::SetDeltaTime(float deltatTime)
{
	currentTime = deltatTime;
	deltaTime = currentTime - lastFrame;
	lastFrame = currentTime;
}
