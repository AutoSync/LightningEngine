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
