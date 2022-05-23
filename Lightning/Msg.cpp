#include "Msg.h"

void Lightning::Msg::Emit(Flow::Flow flag, string message)
{
	switch (flag)
	{
	case Lightning::Flow::INPUT:
		std::cout << "<< " << message << std::endl;
		break;
	case Lightning::Flow::OUTPUT:
		std::cout << ">> " << message << std::endl;
		break;
	case Lightning::Flow::EXIT:
		std::cout << "[Exit] " << message << std::endl;
		exit(0);
		break;
	case Lightning::Flow::ERROR:
		std::cout << "[Error] " << message << std::endl;
		exit(1);
		break;
	case Lightning::Flow::EXPECTED:
		std::cout << "[Expected] " << message << std::endl;
		exit(2);
		break;
	case Lightning::Flow::ABORT:
		std::cout << "[Abort] " << message << std::endl;
		exit(-1);
		break;
	case Lightning::Flow::PRINT:
		std::cout  << message << std::endl;
		break;
	case Lightning::Flow::READ:
		std::cout << message;
		break;
	case Lightning::Flow::PROCESS:
		std::cout << "[%] " << message << std::endl;
		break;
	case Lightning::Flow::WARNING:
		std::cout << "[!WARNING] " << message << std::endl;
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
