#include "Engine.h"

Lightning::Engine::Engine()
{
	window = NULL;
	InitializeWindow(eng_sets);
}

Lightning::Engine::Engine(EngineSettings settings)
{
	eng_sets = settings;
	InitializeWindow(eng_sets);
}

void Lightning::Engine::InitializeEngine()
{
#if defined(WIN32)
	FreeConsole();
#endif
	OnInit();
	OnRender();
	OnTerminate();
}

void Lightning::Engine::Update(){ /* empty implementation */ }

void Lightning::Engine::Start()
{
	Msg::Emit(Flow::INPUT, "[Program Started]");
}

void Lightning::Engine::End()
{
	Msg::Emit(Flow::INPUT, "[Program Finished]");
}

void Lightning::Engine::ExitProgram()
{
	glfwSetWindowShouldClose(window, true);
	Msg::Emit(Flow::PROCESS, "Command Exit");
}

void Lightning::Engine::SetWindowSize(int width, int height)
{
	glfwSetWindowSize(window, width, height);
}

void Lightning::Engine::SetWindowTitle(string _title)
{
	float fps = ceil(1.0 / Time->deltaTime);
	int ifps = (int)fps;
	std::string titleInfo, titteVersion, fpsCount;
	fpsCount = "FPS: " + std::to_string(ifps);
	titleInfo = eng_sets.title;
	titteVersion = eng_sets.version.Text;
	std::string title = titleInfo + titteVersion + fpsCount;

	glfwSetWindowTitle(window, title.c_str());
}

void Lightning::Engine::LateUpdate()
{
}

void Lightning::Engine::SetShowFramerate(bool framerate)
{
	this->framerate = framerate;
}


void Lightning::Engine::OnInit()
{
	//glfwMakeContextCurrent(window);
}

void Lightning::Engine::OnRender()
{
	RenderSettings settings;
	OnRender(settings);
}

void Lightning::Engine::OnRender(RenderSettings settings)
{
	Start();
	while (!glfwWindowShouldClose(window))
	{
		//Initial Program
		SetWindowTitle(eng_sets.title);
		Time->SetDeltaTime((float)glfwGetTime());
		//Initial Data
		glClear(GL_COLOR_BUFFER_BIT);
		//Msg::Emit(Flow::PRINT, Time::deltaTime);
		Update();

		LateUpdate();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	End();
}

void Lightning::Engine::OnTerminate()
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
	Input = new Inputs(window);
}

void Lightning::Engine::SetWindowSizeCallback(GLFWwindow* window, int width, int height)
{
	eng_sets.width = width;
	eng_sets.height = height;
}

