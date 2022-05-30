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

Lightning::SClamp::SClamp()
{
	imin = 0;
	imax = 1;
	fmin = 0.0f;
	fmax = 1.0f;
	dmin = 0.0;
	dmax = 1.0;
}

Lightning::SClamp::SClamp(int MIN, int MAX)
{
	this->imin = MIN;
	this->imax = MAX;
}

Lightning::SClamp::SClamp(float MIN, float MAX)
{
	this->fmin = MIN;
	this->fmax = MAX;
}

Lightning::SClamp::SClamp(double MIN, double MAX)
{
	this->dmin = MIN;
	this->dmax = MAX;
}

Lightning::SClamp::SClamp(const SClamp& clamp)
{
	fmin = clamp.fmin;
	fmax = clamp.fmax;
	dmin = clamp.dmin;
	dmax = clamp.dmax;
}

void Lightning::SClamp::operator=(const SClamp& clamp)
{
	fmin = clamp.fmin;
	fmax = clamp.fmax;
	dmin = clamp.dmin;
	dmax = clamp.dmax;
}

int Lightning::SClamp::clamp(int INPUT)
{
	if (INPUT < imin)
		return imin;
	if (INPUT > imax)
		return imax;
	return INPUT;
}
int Lightning::SClamp::clamp(int INPUT, int MIN, int MAX)
{
	this->imin = MIN;
	this->imax = MAX;
	if (INPUT < imin)
		return imin;
	if (INPUT > imax)
		return imax;
	return INPUT;
}
float Lightning::SClamp::clamp(float INPUT)
{
	if (INPUT < fmin)
		return fmin;
	if (INPUT > fmax)
		return fmax;
	return INPUT;
}
float Lightning::SClamp::clamp(float INPUT, float MIN, float MAX)
{
	this->fmin = MIN;
	this->fmax = MAX;
	if (INPUT < fmin)
		return fmin;
	if (INPUT > fmax)
		return fmax;
	return INPUT;
}
double Lightning::SClamp::clamp(double INPUT)
{
	if (INPUT < dmin)
		return dmin;
	if (INPUT > dmax)
		return dmax;
	return INPUT;
}
double Lightning::SClamp::clamp(double INPUT, double MIN, double MAX)
{
	this->dmin = MIN;
	this->dmax = MAX;
	if (INPUT < dmin)
		return dmin;
	if (INPUT > dmax)
		return dmax;
	return INPUT;
}


