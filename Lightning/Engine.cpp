#include "Engine.h"

Lightning::Engine::Engine()
{
	window = NULL;
	EngineSettings sts;
	InitializeWindow(sts);
}

Lightning::Engine::Engine(EngineSettings settings)
{
	InitializeWindow(settings);
}

void Lightning::Engine::InitializeComponent()
{
#if defined(WIN32)
	FreeConsole();
#endif
	Begin();
	Render();
	End();
}

void Lightning::Engine::Begin()
{
	//glfwMakeContextCurrent(window);
}

void Lightning::Engine::Render()
{
	RenderSettings settings;
	Render(settings);
}

void Lightning::Engine::Render(RenderSettings settings)
{
	BeginPlay();
	while (!glfwWindowShouldClose(window))
	{
		//Initial Data
		glClear(GL_COLOR_BUFFER_BIT);
		Time::SetDeltaTime((float)glfwGetTime());
		//Msg::Emit(Flow::PRINT, Time::deltaTime);
		Update();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	EndPlay();
}

void Lightning::Engine::End()
{
	glfwTerminate();
	Msg::Emit(Flow::EXIT, "[Program Finished]");
}

void Lightning::Engine::InitializeWindow(EngineSettings settings)
{
	Msg::Emit(Flow::PROCESS, "Lightning Engine");
	Msg::When(!glfwInit(), Flow::ERROR, "Failed to Initilize GLFW");

	//Set Opengl version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, settings.version_major);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, settings.version_minor);
	if (settings.core_profile)
		glfwWindowHint(GLFW_OPENGL_COMPAT_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	else
		glfwWindowHint(GLFW_OPENGL_COMPAT_PROFILE, GLFW_OPENGL_FORWARD_COMPAT);
	//Setting Window
	window = glfwCreateWindow(settings.width, settings.height, settings.title, NULL, NULL);
	glfwMakeContextCurrent(window);

	GLenum gl_error = glewInit();
	Msg::When(GLEW_OK != gl_error, Flow::ERROR, "Failed to Initialize GLEW", glfwTerminate);


	Msg::When(!window, Flow::ERROR, "Failed to create Glfw Window");
}

void Lightning::Time::SetDeltaTime(float dt)
{
	currentTime = dt;
	deltaTime = currentTime - lastTime;
	lastTime = currentTime;
}
