#include "lightning-backend.h"

void RenderCommand(int flag, int condition)
{
	switch(flag)
	{
	case LR_DEPTH:
		switch (condition)
		{
		case LDEFAULT:
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
			break;
		case LTRUE:
			glEnable(GL_DEPTH_TEST);
			break;
		case LFALSE:
			glDisable(GL_DEPTH_TEST);
			break;
		case DEPTH_ALWAYS:
			glDepthFunc(GL_ALWAYS);
			break;
		case DEPTH_NEVER:
			glDepthFunc(GL_NEVER);
			break;
		case DEPTH_LESS:
			glDepthFunc(GL_LESS);
			break;
		case DEPTH_EQUAL:
			glDepthFunc(GL_EQUAL);
			break;
		case DEPTH_LEQUAL:
			glDepthFunc(GL_LEQUAL);
			break;
		case DEPTH_GREATER:
			glDepthFunc(GL_GREATER);
			break;
		case DEPTH_NOTEQUAL:
			glDepthFunc(GL_NOTEQUAL);
			break;
		case DEPTH_GEQUAL:
			glDepthFunc(GL_GEQUAL);
			break;
		case DEPTH_NOMASK:
			glDepthMask(GL_FALSE);
			break;
		}
		break;
	case LR_STENCIL:
		break;
	case LR_BLEND:
		break;
	case LR_CULL:
		switch (condition)
		{
		case LDEFAULT:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glFrontFace(GL_CCW);
			break;
		}
		break;

	}
}

glm::vec3 MakeRotate(glm::vec3 rotation)
{
	glm::vec3 vector;
	vector.x = cos(Random::Math::radians(rotation.x) * cos(Random::Math::radians(rotation.y)));
	vector.y = sin(Random::Math::radians(rotation.y));
	vector.z = sin(Random::Math::radians(rotation.x) * cos(Random::Math::radians(rotation.y)));
	return glm::normalize(vector);
}

Console::Console()
{

}

Console::~Console()
{
}

void Console::init()
{
	Start = steady_clock::now();
}

void Console::Log(std::string text)
{
	End = steady_clock::now();
	std::cout << "[" << durationProcess(Start, End) << "] " << text << std::endl;
}

void Console::Msg(std::string text)
{
	std::cout << text;
}

void Console::Now(std::string text)
{
	End = steady_clock::now();
	std::cout << "[" << durationProcess(Start, End) << "] " << text << std::endl;
}

std::string Console::durationProcess(now Start, now End)
{
	duration<double> time = duration_cast<duration<double>>(End - Start);
	return std::to_string(time.count()) + " seconds";
}

