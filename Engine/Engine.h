#pragma once
#define GLEW_STATIC 
#include <GLEW/glew.h> 
#include <GLFW/glfw3.h>

//Tools Engine
#include "Types.h"
#include "System.h"
#include "Msg.h"
#include "Inputs.h"

//Welcome
#define LIGHTNING_WELCOME "2022 (C) LIGHTNING ENGINE"

namespace Lightning
{
	//OpenGL window and context initialization object
	class Engine
	{
	public:
		EngineSettings engine_settings;
		Framerate framerate = Framerate::UNLIMITED;
	private:
		GLFWwindow* window = NULL;
		double fpsLimit = 1.0 / 600;
	public:
		Inputs* Input = new Inputs(window);
		Engine();
		Engine(EngineSettings settings);
		//Call to start the engine
		void InitializeEngine();
		//[Required] Called every frame
		virtual void Update();
		//[Required] Called when starting the program
		virtual void Start();
		//[Required] Called when the program ends
		virtual void End();
		//Last component to be rendered
		virtual void LateUpdate();
		//When program terminate
		virtual void WhenEnd();
	public:
		void ExitProgram();
		void SetWindowSize(int width, int height);
		void SetDisplayVersion(bool enable = false);
		void SetWindowTitle(string title);
		void SetShowFramerate(bool enable = false);
		void SetDoubleframe(bool enable = false);
		void RenderCommand(int flag, int value);
		void SetClearColor(LinearColor clear);
		void SetFramerate(Framerate framerate);
	private:
		void OnInit();
		void OnRender();
		void OnRender(RenderSettings settings);
		void OnTerminate();
		void InitializeWindow(EngineSettings settings);
		void SetWindowSizeCallback(GLFWwindow* window, int width, int height);
		void UpdateTitlebar();
		void UpdateFramerate();
	};
}
