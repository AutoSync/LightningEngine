#pragma once
#define GLEW_STATIC 
#include <GLEW/glew.h> 
#include <GLFW/glfw3.h>

//Tools Engine
#include "System.h"
#include "Msg.h"
#include "Types.h"
#include "Inputs.h"
#include "GameTime.h"

//Welcome
#define LIGHTNING_WELCOME "2022 (C) LIGHTNING ENGINE"

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
		Version version = Version(0, 14, 0, 0, APP_VERSION_STRING);
		int version_major = 3;
		int version_minor = 3;
		bool vsync = false;
		bool core_profile = true;
		bool doubleFrame = false;
		bool framerate = true;
		bool displayVersion = true;
		bool AA = false;
	};

	//OpenGL window and context initialization object
	class Engine
	{
	public:
		EngineSettings engine_settings;
	private:
		GLFWwindow* window = NULL;
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
	private:
		void OnInit();
		void OnRender();
		void OnRender(RenderSettings settings);
		void OnTerminate();
		void InitializeWindow(EngineSettings settings);
		void SetWindowSizeCallback(GLFWwindow* window, int width, int height);
		void UpdateTitlebar();
	};
}
