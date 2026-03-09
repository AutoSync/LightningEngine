#pragma once
struct GLFWwindow;	//Forward declaration

namespace LightningEngine
{
	class Inputs
	{
	private:
		GLFWwindow* window;
		bool hideCursor = false;
	public:
		Inputs(GLFWwindow* wnd);
		bool GetKeyPress(int key);
		bool GetMousePress(int key);
		void SetHideCursor(bool condition = false);
	};
}
