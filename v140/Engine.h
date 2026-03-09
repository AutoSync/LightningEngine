/*
* AutoSync Lightning Engine by Erick Andrade
* 2018 - 2025 - Copyright Lightning Engine - All Rights Reserved
* Engine main window class
*/

#include <glad/glad.h>
#include <GLFW/glfw3.h>

//Tools Engine
#include "Types.h"			//Global scope of enumerators, structs, and classes
#include "Msg.h"			//Formatted message sender
#include "Inputs.h"			//Inputs Class
#include "System.h"			//Global Variables
#include "Match.h"			//Match Functions

//Memory
#include <memory>

//Welcome
#define LIGHTNING_WELCOME "2025 (C) LIGHTNING ENGINE"

namespace LightningEngine
{
	//OpenGL window and context initialization object
	class EngineCore
	{
	public:
		//Engine Settings Data
		EngineSettings settings;							
		//Framerate of the engine
		Framerate framerate = Framerate::UNLIMITED;		
	private:
		//OpenGL window
		GLFWwindow* window = nullptr;
		//Framerate limit
		int limiter = 0;		
		int FPS = 0;
	public:
		//Inputs Class
		std::unique_ptr<Inputs> Input;
		//Constructor Empty
		EngineCore();
		//Constructor with settings
		EngineCore(EngineSettings settings, bool simpleInitialize);
		//Constructor with settings
		EngineCore(EngineSettings settings);
		//Destructor
		~EngineCore(){}
		//[Required] Initializer with InitializeComponent
		virtual void Run() = 0;						
		//Call to start the engine
		void InitializeEngine();						
		// Called every frame to update data
		virtual void Update();			//[ CHANGING FOR PROCESSING CPU DATA ]
		// Called every frame
		virtual void Render();
		//Called when starting the program
		virtual void Start() = 0;
		//Called when the program ends
		virtual void End() = 0;
		//Last component to be rendered
		virtual void LateUpdate();
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
		//Update Title bar informations
		void UpdateTitlebar();
		//Update Framerate
		void SetLimiter();
	protected:
		//Initialize Window
		void InitializeWindow(EngineSettings settings);
	};
}
