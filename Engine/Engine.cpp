#include "Engine.h"

void FixedRender(void(*function)(void))
{
	function();
}

Lightning::Engine::Engine()
{
	window = NULL;
	InitializeWindow(engs);
}

Lightning::Engine::Engine(EngineSettings settings)
{
	engs = settings;
	InitializeWindow(engs);
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
	this->engs.displayVersion = enable;
}

void Lightning::Engine::SetWindowTitle(string _title)
{
	this->engs.title = _title.c_str();
}

void Lightning::Engine::LateUpdate() { /* empty implementation */  }

void Lightning::Engine::FixedUpdate(){ /* empty implementation */ }

void Lightning::Engine::WhenEnd() { /* empty implementation */ }

void Lightning::Engine::SetShowFramerate(bool framerate)
{
	this->engs.displayFPS = framerate;
}

void Lightning::Engine::SetDoubleframe(bool enable)
{
	this->engs.doubleFrame = enable;
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

void Lightning::Engine::SetClearColor(LinearColor clear, int buffer)
{
	SetClearColor(clear);
	RenderCommand(LR_CLEAR, buffer);
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

	//Begin Play
	Start();				
	
	while (!glfwWindowShouldClose(window))
	{
		Time->SetDeltaTime(glfwGetTime());
		UpdateTitlebar();
		SetLimiter();
		
		if (framerate != Framerate::UNLIMITED)
		{
			if (FPS >= limiter)
			{
				Time->SetDeltaTime(glfwGetTime());
				Rendering();
				SetWindowTitle(to_string(FPS));
			}
		}
		else
		{
			Rendering();
		}
	}
	
	//End Play
	End();					
}



void Lightning::Engine::OnTerminate()
{
	WhenEnd();
	glfwTerminate();
	Msg::Emit(Flow::EXIT, "[Program Finished]");
}

void Lightning::Engine::Rendering()
{
	//Render Loop
	Update();
	if (framerate != Framerate::UNLIMITED)
	{
		FixedUpdate();
	}
	//Last Update
	LateUpdate();
	glfwSwapBuffers(window);
	glFlush();
	glfwPollEvents();
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
	engs.width = width;
	engs.height = height;
}

void Lightning::Engine::UpdateTitlebar()
{
	//Buffers text
	std::string titleInfo, titteVersion, fpsCount, space;

	space = " | ";
	double fps = ceil(1.0 / Time->deltaTime);
	FPS = (int)fps;

	titleInfo = engs.title;

	if (engs.displayFPS)
		fpsCount = space + "FPS: " + std::to_string(FPS);
	if (engs.displayVersion)
		titteVersion = engs.version.Text;

	std::string title = titleInfo + titteVersion  + fpsCount;


	glfwSetWindowTitle(window, title.c_str());
}

void Lightning::Engine::SetLimiter()
{ 
	switch (framerate)
	{
	case Lightning::FR15:
		limiter = 15;
		break;
	case Lightning::FR24:
		limiter = 24;
		break;
	case Lightning::FR30:
		limiter = 30;
		break;
	case Lightning::FR60:
		limiter = 60;
		break;
	case Lightning::FR90:
		limiter = 90;
		break;
	case Lightning::FR120:
		limiter = 120;
		break;
	case Lightning::FR240:
		limiter = 240;
		break;
	case Lightning::FR480:
		limiter = 480;
		break;
	case Lightning::UNLIMITED:
		limiter = 0;
		break;
	}
}