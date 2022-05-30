#pragma once
#include <stdlib.h>
#include <iostream>
#include <string>

#include "Types.h"

using namespace std;

namespace Lightning
{
	
	namespace Msg
	{
		//Output a message from a flag
		void Emit(Flow::Flow flag, string message);
		//When the value is true, it returns a message, and a state from a flag
		void When(bool condition, Flow::Flow flag, string message);
		void When(bool condition, Flow::Flow flag, string message, void then(void));

		class Console
		{
		public:
			Console();
			void Init();
			void Log(string message);
		private:
			std::string durationProcess(now Start, now End);
			now Start;
			now End;
		};
	}
}

