#pragma once
#define GLEW_STATIC 
#include <GLEW/glew.h> 
#include <GLFW/glfw3.h>

#if defined(WIN32)
#include <Windows.h>
#endif

//Tools Engine
#include "Msg.h"
#include "Types.h"

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
		int version_major = 4;
		int version_minor = 3;
		bool vsync = false;
		bool core_profile = true;
	};

	//OpenGL window and context initialization object
	class Engine
	{
	public:
		bool ShowConsole = false;
	private:
		GLFWwindow* window = NULL;
	public:
		Engine();
		Engine(EngineSettings settings);
		void InitializeComponent();
		virtual void Update() = 0;
		virtual void BeginPlay() = 0;
		virtual void EndPlay() = 0;
	private:
		void Begin();
		void Render();
		void Render(RenderSettings settings);
		void End();
		void InitializeWindow(EngineSettings settings);
	};

	namespace Time
	{
		static float deltaTime = 0;
		static float currentTime = 0;
		static float lastTime = 0;

		void SetDeltaTime(float dt);
	}
}
