#include "Inputs.h"

Lightning::Inputs::Inputs(GLFWwindow* wnd)
{
	window = wnd;
}

bool Lightning::Inputs::GetKeyPress(int key)
{
	if (glfwGetKey(window, key) == GLFW_PRESS)
		return true;
	else
		return false;
}

bool Lightning::Inputs::GetMousePress(int key)
{
	if (glfwGetMouseButton(window, key) == GLFW_PRESS)
		return true;
	else
		return false;
}

void Lightning::Inputs::SetHideCursor(bool condition)
{
	if(condition)
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	else
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}
