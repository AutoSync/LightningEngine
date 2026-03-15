#include "lightning-frontend.h"

Random::Structs::SWindow ws; // Window Setup
Random::Structs::Context cs; // Window Context
Random::Structs::Framebuffers fb;	//Framebuffer
Random::Structs::MouseEvents me; // Mouse Events

bool isWindowFocus = true;

void size_callback(GLFWwindow* wnd, int width, int height);
void mouse_callback(GLFWwindow* wnd, double mouse_x, double mouse_y);
void scroll_callback(GLFWwindow* wnd, double offset_x, double offset_y);
void focusWindow(GLFWwindow* wnd, int focus);

Lightning::Front::Window::Window()
{
}

Lightning::Front::Window::~Window()
{
}


bool Lightning::Front::Window::init(Random::Structs::SWindow window, Random::Structs::Context context, Random::Structs::Framebuffers framebuffer)
{
	//Initialization Structs
	ws = window;
	cs = context;
	fb = framebuffer;
	glfwInit();
	if (context.minimal_core != true)
	{
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, cs.major_version);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, cs.minor_version);
		if (cs.core)
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		else
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE | GLFW_OPENGL_ANY_PROFILE);
		if (ws.canSize)
			glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
		else
			glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	#ifdef __APPLE__															
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);				//Apple Mac compatibility
	#endif
	}

	GLFWwindow* wnd = glfwCreateWindow(ws.width, ws.height, ws.title.c_str(), NULL, NULL);

	if (!wnd)
	{
		std::cout << "ERROR :(\t GLFW FAILED TO CREATE GLFW WINDOW \n";
		glfwTerminate();
		return false;
	}
	/*********************************** CALLBACKS CUSTOM **************************************/
	glfwMakeContextCurrent(wnd);
	glfwSetFramebufferSizeCallback(wnd, size_callback);
	glfwSetCursorPosCallback(wnd, mouse_callback);
	glfwSetScrollCallback(wnd, scroll_callback);
	glfwSetWindowFocusCallback(wnd, focusWindow);
	if(framebuffer.vsync)
		glfwSwapInterval(1);
#if defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
	GLenum glewerr = glewInit();
#endif 
	if (GLEW_OK != glewerr)
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(glewerr));
		glfwTerminate();
		return false;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.Fonts->AddFontFromFileTTF("Resources/fonts/roboto-regular.ttf", 18);

	ImGui::StyleColorsDark();	//Adicionar um if

	ImGui_ImplGlfw_InitForOpenGL(glfwGetCurrentContext(), true);
	ImGui_ImplOpenGL3_Init(Lightning::Front::Window::glsl_version);
	

	return true;
}

bool Lightning::Front::Window::child(Random::Structs::SWindow ws)
{
	GLFWwindow* cwnd = glfwCreateWindow(ws.width, ws.height, ws.title.c_str(), NULL, glfwGetCurrentContext());
	if(!cwnd)
		return false;

	return true;
}

bool Lightning::Front::Window::release()
{
	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
	return true;
}

bool Lightning::Front::Window::broadcast()
{
	glfwSwapBuffers(glfwGetCurrentContext());
	glFlush();
	glfwPollEvents();
	return true;
}

bool Lightning::Front::Window::getClose()
{
	return !glfwWindowShouldClose(glfwGetCurrentContext());
}

bool Lightning::Front::Window::getWindowFocus()
{
	return isWindowFocus;
}

bool Lightning::Front::Window::renderGUI()
{
	ImGui::Render();
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	return false;
}


GLFWwindow* Lightning::Front::Window::getWindow()
{
	return glfwGetCurrentContext();
}

Random::Structs::MouseEvents Lightning::Front::Window::getMouseEvents()
{
	return me;
}


void size_callback(GLFWwindow* wnd, int width, int height)
{
	ws.width = width;
	ws.height = height;
	glViewport(0, 0, ws.width, ws.height);
	std::cout << "Resized to " << ws.width << "X" << ws.height << std::endl;
}

void mouse_callback(GLFWwindow* wnd, double mouse_x, double mouse_y)
{
	me.position.x = mouse_x;
	me.position.y = mouse_y;
}

void scroll_callback(GLFWwindow* wnd, double offset_x, double offset_y)
{
	me.scroll.x = offset_x;
	me.scroll.y = offset_y;
}		

void focusWindow(GLFWwindow* wnd, int focus)
{
	if (focus)
		isWindowFocus = true;
	else
		isWindowFocus = false;
}


Lightning::Front::Input::Input()
{
}

Lightning::Front::Input::~Input()
{
}

bool Lightning::Front::Input::getKeyPress(EKeyboard key)
{
	if (glfwGetKey(glfwGetCurrentContext(), key) == GLFW_PRESS)
		return true;
	else
		return false;
}

bool Lightning::Front::Input::getMouseButton(EMouse key)
{
	if (glfwGetMouseButton(glfwGetCurrentContext(), key) == GLFW_PRESS)
		return true;
	else
		return false;
}

void Lightning::Front::Input::setShowCursor(bool show, ECursor cursor)
{
	if(show)
		glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	else
		glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}


