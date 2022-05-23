#include "Types.h"

float Lightning::Clampf(float value, float min, float max)
{
	float t;
	if (value >= max)
		t = max;
	else if (value <= min)
		t = min;
	else
		t = value;
	return t;
}
