#include "../../Msg.h"

void LightningEngine::Msg::Emit(Flow::Flow flag, string message)
{
	switch (flag)
	{
	case LightningEngine::Flow::INPUT:
		printf("\x1B[92m<< %s\033[0m\n", message.c_str());
		break;
	case LightningEngine::Flow::OUTPUT:
		printf("\x1B[90m>> %s\033[0m\n", message.c_str());
		break;
	case LightningEngine::Flow::EXIT:
		printf("\033[3;30;44m[EXIT] %s \033[0m\n", message.c_str());
		exit(0);
		break;
	case LightningEngine::Flow::ERROR:
		printf("\033[3;30;41m[ERROR] %s \033[0m\n", message.c_str());
		exit(1);
		break;
	case LightningEngine::Flow::EXPECTED:
		printf("\033[3;30;45m[EXPECTED] %s \033[0m\n", message.c_str());
		exit(2);
		break;
	case LightningEngine::Flow::ABORT:
		printf("\033[3;30;41m[ABORT] %s \033[0m\n", message.c_str());
		exit(-1);
		break;
	case LightningEngine::Flow::PRINT:
		std::cout  << message << std::endl;
		break;
	case LightningEngine::Flow::READ:
		std::cout << message;
		break;
	case LightningEngine::Flow::PROCESS:
		printf("\033[3;30;44m[%%] %s \033[0m\n", message.c_str());
		break;
	case LightningEngine::Flow::WARNING:
		printf("\033[3;30;43m[!] %s \033[0m\n", message.c_str());
		break;
	case LightningEngine::Flow::WARNING_WNL:
		printf("\033[3;30;43m[!] %s \033[0m\n", message.c_str());
		break;
	}
}


void LightningEngine::Msg::When(bool condition, Flow::Flow flag, string message)
{
	if (condition)
	{
		Emit(flag, message);
	}
}

void LightningEngine::Msg::When(bool condition, Flow::Flow flag, string message, void then(void))
{
	if (condition)
	{
		then();
		Emit(flag, message);
	}
}

LightningEngine::Msg::Console::Console()
{
	Init();
}

void LightningEngine::Msg::Console::Init()
{
	Start = steady_clock::now();
}

void LightningEngine::Msg::Console::Log(string message)
{
	End = steady_clock::now();
	string t = "[" + durationProcess(Start, End) + "] " + message;
	Emit(Flow::PRINT, t);
}

std::string LightningEngine::Msg::Console::durationProcess(now Start, now End)
{
	duration<double> time = duration_cast<duration<double>>(End - Start);
	return std::to_string(time.count()) + " seconds";
}
