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
