#pragma once
#define GLEW_STATIC 
#include <GLEW/glew.h> 
#include <GLFW/glfw3.h>

#if defined(WIN32)
#include <Windows.h>
#endif

//Tools Engine
#include "System.h"
#include "Msg.h"
#include "Types.h"
#include "Inputs.h"
#include "GameTime.h"

namespace Lightning
{
	//Has a set of definitions for rendering OpenGL
	struct RenderSettings
	{
		bool Recolor = false;
		V4 color = V4();
	};
	//Initial setup to define window creation
	struct EngineSettings
	{
		int width = 840;
		int height = 480;
		const char* title = "Lightning Engine";
		Version version = Version(0, 10, 0, 0, APP_VERSION_STRING);
		int version_major = 4;
		int version_minor = 5;
		bool vsync = false;
		bool core_profile = true;
		bool framerate = false;
		bool AA = false;
	};
	//OpenGL window and context initialization object
	class Engine
	{
	public:
		bool ShowConsole = false;
	private:
		EngineSettings eng_sets;
		GLFWwindow* window = NULL;
		bool framerate = false;
	public:
		Inputs* Input = new Inputs(window);
		GameTime* Time = new GameTime();
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
	public:
		void ExitProgram();
		void SetWindowSize(int width, int height);
		void SetWindowTitle(string title);
		void SetShowFramerate(bool framerate);
	private:
		void OnInit();
		void OnRender();
		void OnRender(RenderSettings settings);
		void OnTerminate();
		void InitializeWindow(EngineSettings settings);
		void SetWindowSizeCallback(GLFWwindow* window, int width, int height);
	};
}
