#pragma once
#define GLEW_STATIC 
#include <GLEW/glew.h> 
#include <GLFW/glfw3.h>

//Tools Engine
#include "Types.h"			//Global scope of enumerators, structs, and classes
#include "Msg.h"			//Formatted message sender
#include "Inputs.h"			//Inputs Class
#include "System.h"			//Global Variables
#include "Match.h"			//Match Functions

//Welcome
#define LIGHTNING_WELCOME "2022 (C) LIGHTNING ENGINE"

namespace Lightning
{
	//OpenGL window and context initialization object
	class Engine
	{
	public:
		//Engine Settings Data
		EngineSettings engs;							
		//Framerate of the engine
		Framerate framerate = Framerate::UNLIMITED;		
	private:
		//OpenGL window
		GLFWwindow* window = NULL;						
		//Framerate limit
		int limiter = 0;		
		int FPS = 0;
	public:
		//Inputs Class
		Inputs* Input = new Inputs(window);				
		//Constructor Empty
		Engine();										
		//Constructor with settings
		Engine(EngineSettings settings);
		//Destructor
		~Engine(){ delete this; }
		//[Required] Initializer with InitializeComponent
		virtual void Run() = 0;						
		//Call to start the engine
		void InitializeEngine();						
		//Called every frame
		virtual void Update();
		//Called when starting the program
		virtual void Start();
		//Called when the program ends
		virtual void End();
		//Last component to be rendered
		virtual void LateUpdate();
		//Set Render Fixed
		virtual void FixedUpdate();
		//When program terminate
		virtual void WhenEnd();
	public:
		//Called when the window is resized
		void ExitProgram();
		//Called when the window is resized
		void SetWindowSize(int width, int height);
		//Set Display Version
		void SetDisplayVersion(bool enable = false);
		//Set Window Title
		void SetWindowTitle(string title);
		//Set Display Framerate
		void SetShowFramerate(bool enable = false);
		//Set Double frame
		void SetDoubleframe(bool enable = false);
		//Set Render Commands
		void RenderCommand(int flag, int value);
		//Set Clear Color
		void SetClearColor(LinearColor clear);
		//Set Clear Color
		void SetClearColor(LinearColor clear, int buffer);
		//Set Framerate Limit
		void SetFramerate(Framerate framerate);
		// Update Time
		void Timer();
	private:
		//Initialze engine
		void OnInit();
		//Render Loop
		void OnRender();
		//Render Loop
		void OnRender(RenderSettings settings);
		//Terminate engine
		void OnTerminate();
		//Rendering
		void Rendering();
		//Initialize Window
		void InitializeWindow(EngineSettings settings);
		//Update Title bar informations
		void UpdateTitlebar();
		//Update Framerate
		void SetLimiter();
	};
}
