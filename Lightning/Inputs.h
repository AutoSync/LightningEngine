#pragma once
#include <GLFW/glfw3.h>
namespace Lightning
{
	class Inputs
	{
	private:
		GLFWwindow* window = NULL;
	public:
		Inputs(GLFWwindow* wnd);
		bool GetKeyPress(int key);
	};
}
