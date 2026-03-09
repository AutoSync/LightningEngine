#include "../../Engine.h"

void FixedRender(void(*function)(void))
{
	function();
}

LightningEngine::EngineCore::EngineCore()
{
	/* Empty */
}

LightningEngine::EngineCore::EngineCore(EngineSettings settings, bool simpleInitialize)
{
	if(simpleInitialize)
		window = nullptr;
	InitializeWindow(settings);
}

LightningEngine::EngineCore::EngineCore(EngineSettings settings)
{
	window = nullptr;
	settings = settings;
	InitializeWindow(settings);
}

void LightningEngine::EngineCore::InitializeEngine()
{
	OnInit();
	OnRender();
	OnTerminate();
}

void LightningEngine::EngineCore::Update(){ /* empty implementation */ }
void LightningEngine::EngineCore::LateUpdate() { /* empty implementation */ }
void LightningEngine::EngineCore::Render(){ /* empty implementation */ }

void LightningEngine::EngineCore::Start()
{
	Msg::Emit(Flow::INPUT, "[Program Started]");
}

void LightningEngine::EngineCore::End()
{
	Msg::Emit(Flow::INPUT, "[Program Finished]");
}

void LightningEngine::EngineCore::ExitProgram()
{
	glfwSetWindowShouldClose(window, true);
	Msg::Emit(Flow::PROCESS, "Command Exit");
}

void LightningEngine::EngineCore::SetWindowSize(int width, int height)
{
	glfwSetWindowSize(window, width, height);
}

void LightningEngine::EngineCore::SetDisplayVersion(bool enable)
{
	this->settings.displayVersion = enable;
}

void LightningEngine::EngineCore::SetWindowTitle(string _title)
{
	this->settings.title = _title.c_str();
}

void LightningEngine::EngineCore::WhenEnd() { /* empty implementation */ }

void LightningEngine::EngineCore::SetShowFramerate(bool framerate)
{
	this->settings.displayFPS = framerate;
}

void LightningEngine::EngineCore::SetDoubleframe(bool enable)
{
	this->settings.doubleFrame = enable;
}

void LightningEngine::EngineCore::RenderCommand(int flag, int value)
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

void LightningEngine::EngineCore::SetClearColor(LinearColor clear)
{
	glClearColor(clear.r, clear.g, clear.b, clear.a);
}

void LightningEngine::EngineCore::SetClearColor(LinearColor clear, int buffer)
{
	SetClearColor(clear);
	RenderCommand(LR_CLEAR, buffer);
}

void LightningEngine::EngineCore::SetFramerate(Framerate framerate)
{
	this->framerate = framerate;
}

void LightningEngine::EngineCore::Timer()
{
	Time->SetDeltaTime(glfwGetTime());
}

void LightningEngine::EngineCore::OnInit() { /* empty implementation */ }

void LightningEngine::EngineCore::OnRender()
{
	RenderSettings settings;
	OnRender(settings);
}

void LightningEngine::EngineCore::OnRender(RenderSettings settings)
{
	Msg::Emit(Flow::OUTPUT, "Start Engine");

	//Begin Play
	Start();
	//Render Loop
	Rendering();
	//End Play
	End();					
}


void LightningEngine::EngineCore::OnTerminate()
{
	WhenEnd();
	glfwTerminate();
	Msg::Emit(Flow::EXIT, "[Program Finished]");
}

void LightningEngine::EngineCore::Rendering()
{
	//Render Loop
	while (!glfwWindowShouldClose(window))
	{
		//Set Time
		Timer();

		//Update Title bar informations
		UpdateTitlebar();
		
		// Processing data
		Update();

		//Render
		Render();

		//Last Update
		LateUpdate();

		//Swap Buffers and Poll Events
		glfwSwapBuffers(window);
		glFlush();
		glfwPollEvents();
	}
}

void LightningEngine::EngineCore::InitializeWindow(EngineSettings settings)
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

	Msg::When(!window, Flow::ERROR, "Failed to create Glfw Window");
	Input = std::make_unique<Inputs>(window);

	//GLAD init
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		Msg::Emit(Flow::ERROR, "Failed to initialize GLAD");
		glfwTerminate();
	}
	else
	{
		Msg::Emit(Flow::OUTPUT, "OpenGL Version: " + std::string((const char*)glGetString(GL_VERSION)));
		Msg::Emit(Flow::OUTPUT, "GLSL Version: " + std::string((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION)));
		Msg::Emit(Flow::OUTPUT, "Vendor: " + std::string((const char*)glGetString(GL_VENDOR)));
		Msg::Emit(Flow::OUTPUT, "Renderer: " + std::string((const char*)glGetString(GL_RENDERER)));
	}
}

void LightningEngine::EngineCore::UpdateTitlebar()
{
	//Buffers text
	std::string titleInfo, titteVersion, fpsCount, space, delta;

	space = " | ";
	double fps = ceil(1.0 / Time->deltaTime);
	FPS = (int)fps;
	delta = std::to_string(Time->deltaTime);

	titleInfo = settings.title;

	if (settings.displayFPS)
		fpsCount = space + "FPS: " + std::to_string(FPS) + space + "dt: " + delta;
	if (settings.displayVersion)
		titteVersion = settings.version.Text;

	std::string title = titleInfo + titteVersion  + fpsCount;


	glfwSetWindowTitle(window, title.c_str());
}

void LightningEngine::EngineCore::SetLimiter()
{ 
	switch (framerate)
	{
	case LightningEngine::FR15:
		limiter = 15;
		break;
	case LightningEngine::FR24:
		limiter = 24;
		break;
	case LightningEngine::FR30:
		limiter = 30;
		break;
	case LightningEngine::FR60:
		limiter = 60;
		break;
	case LightningEngine::FR90:
		limiter = 90;
		break;
	case LightningEngine::FR120:
		limiter = 120;
		break;
	case LightningEngine::FR240:
		limiter = 240;
		break;
	case LightningEngine::FR480:
		limiter = 480;
		break;
	case LightningEngine::UNLIMITED:
		limiter = 0;
		break;
	}
}