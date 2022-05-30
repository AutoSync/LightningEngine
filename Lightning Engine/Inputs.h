#pragma once
#include <GLFW/glfw3.h>
namespace Lightning
{
	class Inputs
	{
	private:
		GLFWwindow* window = NULL;
		bool hideCursor = false;
	public:
		Inputs(GLFWwindow* wnd);
		bool GetKeyPress(int key);
		bool GetMousePress(int key);
		void SetHideCursor(bool condition = false);
	};
}
