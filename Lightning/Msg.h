#pragma once
#include <stdlib.h>
#include <iostream>
#include <string>

using namespace std;

namespace Lightning
{
	namespace Flow
	{
		enum Flow
		{
			INPUT, OUTPUT, EXIT, ERROR, EXPECTED, ABORT, PRINT, READ, PROCESS, WARNING
		};
	}
	namespace Colors
	{
		enum ConsoleColor
		{
			BLACK = '\33', RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE,
			BRIGHT_BLACK = 90, BRIGHT_RED, BRIGHT_GREEN, BRIGH_YELLOW, BRIGHT_BLUE,
			BRIGHT_MAGENTA, BRIGHT_CYAN, BRIGHT_WHITE
		};
	}

	namespace Msg
	{
		//Output a message from a flag
		void Emit(Flow::Flow flag, string message);

		//When the value is true, it returns a message, and a state from a flag
		void When(bool condition, Flow::Flow flag, string message);
		void When(bool condition, Flow::Flow flag, string message, void then(void));

	}
	
}

