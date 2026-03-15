#include "Element_Start.h"
#include <windows.h>

//Prints stack of events with elapsed time
// ************************************************************************************************														//Set Delta Time
void setTime();
float getDeltaTimeSeconds();
void setDeltaTime();
void FramerateLimit(int value);													//Limit FPS
void display_fps(GLFWwindow* window);											//Show FPS in Title Bar
void framebuffer_size_callback(GLFWwindow* window, int width, int height);		//Window function
void processInput(GLFWwindow* window);											//Key Press
void mouse_callback(GLFWwindow* window, double location_X, double location_Y);	//Mouse Input
void scroll_callback(GLFWwindow* window, double offset_X, double offset_Y);		//Scroll Input
//#####################################################################################
int screen_width = 800;															//Resolution X
int screen_height = 640;														//Resolution Y
string title_bar = "LIGHTNING ENGINE 0.11.0 | ";								//Title App

Print ps;																		//Print Screen
CameraObject FirstCamera(glm::vec3(0.0f,1.0f,3.0f));							//Camera placement
CameraController CurrentCamera(FirstCamera);									//Camera Controller
float Time, deltaTime, lastFrame;												//Time
float lastX = screen_width / 2;													//Mouse Location in X
float lastY = screen_height / 2;												//Mouse Location in Y
bool fistMouse = true;

bool isMoving = false;															//Enable render when is moving false

int MAX_FPS_COUNT = 90;															//MAX FPS COUNT
int main()
{
	ps.Log("Inicializando Lightning Engine 0.11.0");
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);
	#ifdef __APPLE__															//Apple Mac compatibility
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	#endif

	GLFWwindow* window = glfwCreateWindow(screen_width, screen_height, title_bar.c_str(), NULL, NULL);
	if (window == NULL)
	{
		cout << "Failed to create GLFW window";
		glfwTerminate();
		return -1;
	}
	// Log role events
	glfwMakeContextCurrent(window);											//Make this window current
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);		//Resize the window
	glfwSetCursorPosCallback(window, mouse_callback);						//Register Mouse Callback
	glfwSetScrollCallback(window, scroll_callback);							//Register Scroll Callback
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	//Set show mouse cursor

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "FAILED to Initilize GLAD";
		return -1;
	}
	ps.Log("Inicialização Completa", 1);
	glEnable(GL_DEPTH_TEST);												//Depth test
	glDepthFunc(GL_LESS);													//Depth type
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	stbi_set_flip_vertically_on_load(true);									//Flip Textures
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//Objects
	ps.Log("Criando Luzes na Cena");
	LightDirectional sun;
	sun.setDiffuse(1.0f);
	sun.setAmbient(0.79f, 0.82f, 0.95f);
	sun.setSpecular(0.25);
	sun.setLightDirection(-0.2f, -1.0f, -0.3f);
	glm::vec4 Environment = glm::vec4(sun.getAmbient(), 1.0);

	ps.Log("Carregando Shaders");
	Shader shader("EngineContents/Shaders/MaterialSample.vert", "EngineContents/Shaders/MaterialSample.frag");
	Shader outliner("EngineContents/Shaders/MaterialSample.vert", "EngineContents/Shaders/ObjectOutline.frag");
	ps.Log("Shaders Carregados", 1);

	ps.Log("Carregando Modelos 3D");
	Model warehouse[] = 
	{
		Model ("EngineContents/Meshes/Scene_Test/Floor.obj"),
		Model ("EngineContents/Meshes/Scene_Test/wall.obj"),
		Model ("EngineContents/Meshes/Scene_Test/roof.obj"),
		Model ("EngineContents/Meshes/Scene_test/Barrel.obj")
	};
	//Model lines("EngineContents/Meshes/Scene_Test/lines.obj"); ps.Log("modelo 4");
	ps.Log("Modelos 3D Carregados", 1);

	
	// Pespectiva 
	glm::mat4 projection = glm::perspective(glm::radians(CurrentCamera.ActiveCamera.Zoom), (float)screen_width / (float)screen_height, 0.1f, 100.0f);
	while (!glfwWindowShouldClose(window))									//Render Loop
	{
		//Set Delta Time
		setTime();
		setDeltaTime();
		FramerateLimit(MAX_FPS_COUNT);
		// Input Keyboard/Mouse
		processInput(window);
		display_fps(window);
		
	
		// Render
		//glClearColor(0.79f, 0.82f, 0.95f, 1.0f);							// Background Color
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = CurrentCamera.ActiveCamera.GetViewMatrix();

		glClearColor(Environment.x, Environment.y, Environment.z, Environment.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		
		
		outliner.use();
		outliner.setMat4("projection", projection);
		outliner.setMat4("view", view);

		shader.use();
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);

		shader.setVec3("ViewLocation", CurrentCamera.ActiveCamera.Location);
		shader.setVec3("ldir.direction", sun.getLightDirection());
		shader.setVec3("ldir.diffuse", sun.getDiffuse());
		shader.setVec3("ldir.specular", sun.getSpecular());
		shader.setVec3("ldir.ambient", sun.getAmbient()* 0.2f);
		shader.setFloat("material.shininess", 64.0f);

		for (unsigned int i = 0; i < 3; i++)
		{
			glStencilMask(0x00);
			model = glm::translate(model, glm::vec3(0.0f));
			model = glm::scale(model, glm::vec3(1.0f));
			shader.setMat4("model", model);
			warehouse[i].Draw(shader);
		}
		glStencilFunc(GL_ALWAYS, 1,0xFF);
		glStencilMask(0xFF);
		model = glm::translate(model, glm::vec3(0.0f));
		model = glm::scale(model, glm::vec3(1.0f));
		shader.setMat4("model", model);
		warehouse[3].Draw(shader);

		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00);
		glDisable(GL_DEPTH_TEST);
		outliner.use();
		float scale = 1.05;
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f));
		model = glm::scale(model, glm::vec3(scale, scale, scale));
		outliner.setMat4("model", model);
		warehouse[3].Draw(outliner);
		glStencilMask(0xFF);
		glEnable(GL_DEPTH_TEST);

		//glfwSwapBuffers(window);
		glFlush();
		glfwPollEvents();
		
	}
	glfwTerminate();
	return 0;
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	std::cout << "Resized to " << width << "X" << height << std::endl;
}

void setTime()
{
	float Time = (float)glfwGetTime();
}
float getDeltaTimeSeconds()
{
	return deltaTime;
}

void setDeltaTime()
{
	float CurrentFrame = (float)glfwGetTime();
	deltaTime = CurrentFrame - lastFrame;
	lastFrame = CurrentFrame;
}

void FramerateLimit(int value)
{
	Sleep((1000.0 * 1.0) / value);
}

void display_fps(GLFWwindow* window)
{
	float FPS = 1.0/ deltaTime;
	int ifps = (int)FPS;
	std::string fpsCount;
	fpsCount = "FPS: " + std::to_string(ifps);
	std::string title = title_bar + fpsCount;
	glfwSetWindowTitle(window, title.c_str());
}

void processInput(GLFWwindow* window)
{
	int mouse_right = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2);
	int mouse_left = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1);
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
	// Keyboard Events
	if (mouse_right == GLFW_PRESS)
	{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			CurrentCamera.ActiveCamera.ProcessKeyboard(FORWARD, getDeltaTimeSeconds());
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			CurrentCamera.ActiveCamera.ProcessKeyboard(BACKWARD, getDeltaTimeSeconds());
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			CurrentCamera.ActiveCamera.ProcessKeyboard(LEFT, getDeltaTimeSeconds());
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			CurrentCamera.ActiveCamera.ProcessKeyboard(RIGHT, getDeltaTimeSeconds());
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
			CurrentCamera.ActiveCamera.ProcessKeyboard(UP, getDeltaTimeSeconds());
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
			CurrentCamera.ActiveCamera.ProcessKeyboard(DOWN, getDeltaTimeSeconds());
	}
	else
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void mouse_callback(GLFWwindow* window, double location_X, double location_Y)
{
	int mouse_right = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
	int mouse_left = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	if (fistMouse)
	{
			lastX = (float)location_X;
			lastY = (float)location_Y;
			fistMouse = false;
	}
	float Move_X = location_X - lastX;
	float Move_Y = lastY - location_Y;
	lastX = location_X;
	lastY = location_Y;

	if (mouse_right == GLFW_PRESS)
		CurrentCamera.ActiveCamera.ProcessMouseMovement(Move_X, Move_Y);
}

void scroll_callback(GLFWwindow* window, double offset_X, double offset_Y)
{
	CurrentCamera.ActiveCamera.ProcessMouseScroll((float)offset_Y);
}