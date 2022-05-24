#include "Types.h"

float Lightning::IncrementRangef(char signal, float target, float value, float min, float max)
{
	if (target > max)
		target = max;
	else if (target < min)
		target = min;
	else
	{
		switch (signal)
		{
		case '+':
			target += value;
			break;
		case '-':
			target -= value;
			break;
		}
	}
	return target;	
}

glm::vec3 Lightning::MakeRotate(glm::vec3 rotation)
{
	glm::vec3 vector;
	vector.x = cos(radians(rotation.x) * cos(radians(rotation.y)));
	vector.y = sin(radians(rotation.y));
	vector.z = sin(radians(rotation.x) * cos(radians(rotation.y)));
	return glm::normalize(vector);
}

float Lightning::radians(float degrees)
{
	return degrees * LIGHTNING_RADIANS;
}

float Lightning::degrees(float radians)
{
	return radians * LIGHTNING_DEGREES;
}



