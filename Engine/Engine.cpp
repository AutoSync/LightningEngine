#include "Engine.h"

Lightning::Engine::Engine()
{
	window = NULL;
	InitializeWindow(engine_settings);
}

Lightning::Engine::Engine(EngineSettings settings)
{
	engine_settings = settings;
	InitializeWindow(engine_settings);
}

void Lightning::Engine::InitializeEngine()
{
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

void Lightning::Engine::SetDisplayVersion(bool enable)
{
	this->engine_settings.displayVersion = enable;
}

void Lightning::Engine::SetWindowTitle(string _title)
{
	this->engine_settings.title = _title.c_str();
}

void Lightning::Engine::LateUpdate() { /* empty implementation */  }

void Lightning::Engine::WhenEnd() { /* empty implementation */ }

void Lightning::Engine::SetShowFramerate(bool framerate)
{
	this->engine_settings.displayFPS = framerate;
}

void Lightning::Engine::SetDoubleframe(bool enable)
{
	this->engine_settings.doubleFrame = enable;
}

void Lightning::Engine::RenderCommand(int flag, int value)
{
	switch (flag)
	{
	case LR_DEPTH:
		switch (value)
		{
		case LDEFAULT:
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
			break;
		case LTRUE:
			glEnable(GL_DEPTH_TEST);
			break;
		case LFALSE:
			glDisable(GL_DEPTH_TEST);
			break;
		case DEPTH_ALWAYS:
			glDepthFunc(GL_ALWAYS);
			break;
		case DEPTH_NEVER:
			glDepthFunc(GL_NEVER);
			break;
		case DEPTH_LESS:
			glDepthFunc(GL_LESS);
			break;
		case DEPTH_EQUAL:
			glDepthFunc(GL_EQUAL);
			break;
		case DEPTH_LEQUAL:
			glDepthFunc(GL_LEQUAL);
			break;
		case DEPTH_GREATER:
			glDepthFunc(GL_GREATER);
			break;
		case DEPTH_NOTEQUAL:
			glDepthFunc(GL_NOTEQUAL);
			break;
		case DEPTH_GEQUAL:
			glDepthFunc(GL_GEQUAL);
			break;
		case DEPTH_NOMASK:
			glDepthMask(GL_FALSE);
			break;
		}
		break;
	case LR_STENCIL:
		break;
	case LR_BLEND:
		break;
	case LR_CULL:
		switch (value)
		{
		case LDEFAULT:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glFrontFace(GL_CCW);
			break;
		}
		break;
	case LR_CLEAR:
		glClear(value);

	}
}

void Lightning::Engine::SetClearColor(LinearColor clear)
{
	glClearColor(clear.r, clear.g, clear.b, clear.a);
}

void Lightning::Engine::SetFramerate(Framerate framerate)
{
	this->framerate = framerate;
}

void Lightning::Engine::OnInit() { /* empty implementation */ }

void Lightning::Engine::OnRender()
{
	RenderSettings settings;
	OnRender(settings);
}

void Lightning::Engine::OnRender(RenderSettings settings)
{
	Msg::Emit(Flow::OUTPUT, "Start Engine");
	Start();				//Begin Play
	
	while (!glfwWindowShouldClose(window))
	{
		//Initial Program
		UpdateTitlebar();
		Time->SetDeltaTime(glfwGetTime());
		UpdateFramerate();

		Update();			//Render Loop

		LateUpdate();
		glFlush();
		glfwPollEvents();
	}
	End();					//End Play
}

void Lightning::Engine::OnTerminate()
{
	WhenEnd();
	glfwTerminate();
	Msg::Emit(Flow::EXIT, "[Program Finished]");
}

void Lightning::Engine::InitializeWindow(EngineSettings settings)
{
	Msg::Emit(Flow::PRINT, LIGHTNING_WELCOME);
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
	engine_settings.width = width;
	engine_settings.height = height;
}

void Lightning::Engine::UpdateTitlebar()
{
	//Buffers text
	std::string titleInfo, titteVersion, fpsCount, space;

	space = " | ";
	float fps = ceil(1.0 / Time->deltaTime);
	int ifps = (int)fps;

	titleInfo = engine_settings.title;

	if (engine_settings.displayFPS)
		fpsCount = space + "FPS: " + std::to_string(ifps);
	if (engine_settings.displayVersion)
		titteVersion = engine_settings.version.Text;

	std::string title = titleInfo + titteVersion  + fpsCount;


	glfwSetWindowTitle(window, title.c_str());
}

void Lightning::Engine::UpdateFramerate()
{
	glfwSwapBuffers(window);
}