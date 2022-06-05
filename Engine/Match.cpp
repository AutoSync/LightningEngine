#include "Match.h"

void Lightning::When(bool condition, void(*function)(void))
{
	if (condition)
		function();
}

void Lightning::Expected(bool condition, void(*function)(void), const char* message)
{
	if (!condition)
		Msg::Emit(Flow::EXPECTED, message);
	else
		function();
}
