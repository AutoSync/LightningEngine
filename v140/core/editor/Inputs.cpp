#include "../../Inputs.h"
#include <GLFW/glfw3.h>

LightningEngine::Inputs::Inputs(GLFWwindow* wnd)
{
	window = wnd;
}

bool LightningEngine::Inputs::GetKeyPress(int key)
{
	if (glfwGetKey(window, key) == GLFW_PRESS)
		return true;
	else
		return false;
}

bool LightningEngine::Inputs::GetMousePress(int key)
{
	if (glfwGetMouseButton(window, key) == GLFW_PRESS)
		return true;
	else
		return false;
}

void LightningEngine::Inputs::SetHideCursor(bool condition)
{
	if(condition)
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	else
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}
