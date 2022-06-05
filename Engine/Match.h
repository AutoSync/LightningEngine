#pragma once
#include "Msg.h"
namespace Lightning
{
	static void When(bool condition, void(*function)(void));
	static void Expected(bool condition, void(*function)(void), const char* message);
}
