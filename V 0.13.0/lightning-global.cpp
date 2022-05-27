#include "lightning-global.h"

// Variaveis globais do Programa

static bool bShowFPS = true, bShowMousePos = true, bShowTime = true;

void setInfoTitleBar(bool ShowFPS, bool ShowMousePos)
{
	bShowFPS = ShowFPS;
	bShowMousePos = ShowMousePos;
}

void showInfoTitleBar(GLFWwindow* wnd, Random::Structs::SWindow window, const Random::Structs::MouseEvents& mouse)
{
	text title, FPS, MousePos, TimeCount;
	Time.setTime(glfwGetTime());
	title = window.title;
	if (bShowFPS)
	{
		float dt = 1.0 / (float)Time.deltaTime;
		FPS = "FPS: " + std::to_string((int)dt);
	}
	else
		FPS = "";
	if (bShowMousePos)
		MousePos = " Mouse X:" + std::to_string((int)mouse.position.x) + " Mouse Y:" + std::to_string((int)mouse.position.y)
		+ " Scroll Y:" + std::to_string((int)mouse.scroll.y);
	else
		MousePos = "";
	if (bShowTime)
	{
		int tm = (int)Time.getTimeSeconds();
		TimeCount = " Time: " + std::to_string((float)Time.getTimeSeconds()) + "s";
	}
	else
		TimeCount = "";
	window.title = window.title + FPS + MousePos + TimeCount;
	glfwSetWindowTitle(wnd, window.title.c_str());
}

void InputListener(void(*fn)(GLFWwindow* wnd))
{
	
}

void setShading(Shader* shader, solidtext local, int level)
{
	shader->setInt(local, level);
}
