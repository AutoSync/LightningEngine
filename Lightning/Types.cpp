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
			return target;
		case '-':
			target -= value;
			return target;
		}
	}
	
}
