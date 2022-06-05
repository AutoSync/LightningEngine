#include "Msg.h"

void Lightning::Msg::Emit(Flow::Flow flag, string message)
{
	switch (flag)
	{
	case Lightning::Flow::INPUT:
		printf("\x1B[92m<< %s\033[0m\n", message.c_str());
		break;
	case Lightning::Flow::OUTPUT:
		printf("\x1B[90m>> %s\033[0m\n", message.c_str());
		break;
	case Lightning::Flow::EXIT:
		printf("\x1B[94m[EXIT] %s\033[0m\n", message.c_str());
		exit(0);
		break;
	case Lightning::Flow::ERROR:
		printf("\x1B[31m[ERROR] %s\033[0m\n", message.c_str());
		exit(1);
		break;
	case Lightning::Flow::EXPECTED:
		printf("\x1B[33m[EXPECTED] %s\033[0m\n", message.c_str());
		exit(2);
		break;
	case Lightning::Flow::ABORT:
		printf("\x1B[31m[ABORT] %s\033[0m\n", message.c_str());
		exit(-1);
		break;
	case Lightning::Flow::PRINT:
		std::cout  << message << std::endl;
		break;
	case Lightning::Flow::READ:
		std::cout << message;
		break;
	case Lightning::Flow::PROCESS:
		printf("\x1B[34m[%] %s\033[0m\n", message.c_str());
		break;
	case Lightning::Flow::WARNING:
		printf("\x1B[93m[!] %s\033[0m\n", message.c_str());
		break;
	case Lightning::Flow::WARNING_WNL:
		printf("\x1B[93m[!] %s\033[0m\n", message.c_str());
		break;
	default:
		break;
	}
}


void Lightning::Msg::When(bool condition, Flow::Flow flag, string message)
{
	if (condition)
	{
		Emit(flag, message);
	}
}

void Lightning::Msg::When(bool condition, Flow::Flow flag, string message, void then(void))
{
	if (condition)
	{
		then();
		Emit(flag, message);
	}
}

Lightning::Msg::Console::Console()
{
	Init();
}

void Lightning::Msg::Console::Init()
{
	Start = steady_clock::now();
}

void Lightning::Msg::Console::Log(string message)
{
	End = steady_clock::now();
	string t = "[" + durationProcess(Start, End) + "] " + message;
	Emit(Flow::PRINT, t);
}

std::string Lightning::Msg::Console::durationProcess(now Start, now End)
{
	duration<double> time = duration_cast<duration<double>>(End - Start);
	return std::to_string(time.count()) + " seconds";
}
