// 2020 AutoSync - LIGHTNING ENGINE (--,) ERICK ANDRADE ABRIL DE 2020

#include "StaterMotor.h" // Carrega todas as bibliotecas da engine
#define INITIAL_SCREEN_X 800	// Initial Screen Width
#define INITIAL_SCREEN_Y 600	// Initial Screen Height

//Prints stack of events with elapsed time
// ************************************************************************************************
void framebuffer_size_callback(GLFWwindow* window, int width, int height);		//Window function
void display_fps(GLFWwindow* window);											//Show FPS in Title Bar
void processInput(GLFWwindow* window);											//Key Press
void mouse_callback(GLFWwindow* window, double location_X, double location_Y);	//Mouse Input
void scroll_callback(GLFWwindow* window, double offset_X, double offset_Y);		//Scroll Input

//GLOBAL VARIABLES
// ************************************************************************************************
Print print;												//Print console Logs and times 
const char APP_TITLE[] = "LIGHTNING ENGINE ";				// Tittle of application
const char APP_VERSION[] = "V 0.10.0 DEV PREVIEW  ";		// Version of application
glm::vec3 LightLocation(0.25f, 1.0f, 1.0f);					//Light
float deltaTime = 0.0f;										//Render time range
float lastFrame = 0.0f;										//Last Render Time
CameraObject Camera(glm::vec3(0.0f, 5.0f, 20.0f));			//Camera World
CameraObject CurrentCamera = Camera;						//Active Camera
float lastX = INITIAL_SCREEN_X / 2;							//Last Mouse Location in X 
float lastY = INITIAL_SCREEN_Y / 2;							//Last Mouse Location in Y
bool fistMouse = true;

// Main
// ************************************************************************************************
int main() {
	print.msg("Bem Vindo a Lightning Engine 0.10");
	print.Log("Inicializando o OpenGl");

	glfwInit();																	//Initialize OPENGL
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);								//Major Suport OpenGL version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);								//Minor Support OpenGL version
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);				//OpenGl Profile
	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);								//Turn off the double buffer increasing the FPS
	#ifdef __APPLE__															//Apple Mac compatibility
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	#endif

	GLFWwindow* window = glfwCreateWindow(INITIAL_SCREEN_X, INITIAL_SCREEN_Y,"LIGHTNING ENGINE 0.10.0", NULL, NULL);
	if (window == NULL)
	{
		print.Log("Failed to create GLFW window",1);
		glfwTerminate();
		return -1;
	}
	// Log role events
	glfwMakeContextCurrent(window);											//Make this window current
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);		//Resize the window
	glfwSetCursorPosCallback(window, mouse_callback);						//Register Mouse Callback
	glfwSetScrollCallback(window, scroll_callback);							//Register Scroll Callback
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);			//Set show mouse cursor

	//Load pointers with GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		print.Log("FAILED to Initilize GLAD", 1);
		return -1;
	}
	//Global OpenGL State
	glEnable(GL_DEPTH_TEST);
	print.Log("OpenGl Inicializado!", 1);
	/********************************************************************************/

	/*################################### SHADER ###################################*/
	print.Log("Carregando Elementos do Cenario");
	print.Log("Shaders ");
	LightningShader Shader[]
	{
		LightningShader ("EngineContents/Shaders/BoatScene.vert", "EngineContents/Shaders/BoatScene.frag"),
		LightningShader("EngineContents/Shaders/LightSource.vert","EngineContents/Shaders/LightSource.frag")
	};
	print.Log("Shaders Completo", 1);

	stbi_set_flip_vertically_on_load(true);
	print.Log("Geometrias");
	Model Geometries[] =
	{
		Model("EngineContents/Mesh/FishingBoat/boat.obj"),
		Model("EngineContents/Mesh/EngineMeshes/LampBulb.obj")
	};

	print.Log("Geometrias carregadas", 1);
	
	LightDirectional sun;
	//sun.setDiffuse(1.0f, 0.97f, 0.85f);
	sun.setDiffuse(1.0f);

	sun.setLightDirection(-0.2f, -1.0f, -0.3f);
	

	print.msg("Elementos da cena Completo");
	//Perspective projection
	glm::mat4 projection = glm::perspective(glm::radians(Camera.Zoom), (float)INITIAL_SCREEN_X / (float)INITIAL_SCREEN_Y, 0.1f, 100.0f);
	//RENDER LOOP
	while (!glfwWindowShouldClose(window))
	{
		//Tick
		float Time = (float)glfwGetTime();
		float CurrentFrame = Time;
		deltaTime = CurrentFrame - lastFrame;
		lastFrame = CurrentFrame;

		// Input Keyboard/Mouse
		processInput(window);
		// Render
		
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Background Color
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		

		//Scene Shader
		Shader[1].use();
		//glm::mat4 projection = glm::perspective(glm::radians(CurrentCamera.Zoom), (float)800 / (float)640, 0.1f, 100.0f);
		glm::mat4 view = CurrentCamera.GetViewMatrix();
		Shader[1].setMat4("projection", projection);
		Shader[1].setMat4("view", view);
		//Sun Color
		/* 
		scene.setVec3("ldir.direction",sun.getLightDirection());
		scene.setVec3("ldir.diffuse", sun.getDiffuse());
		scene.setVec3("ldir.specular", sun.getSpecular());
		*/

		//sun.Render(Shader[1]);
		Shader[1].setFloat("material.shininess", 16.0f);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f,sin(Time + 2.0f),0.0f));
		model = glm::rotate(model, sin(-Time) * glm::radians(7.5f), glm::vec3(0.0f,0.0f,1.0f));
		model = glm::rotate(model, sin(-Time) * glm::radians(5.5f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f));
		Shader[1].setMat4("model", model);
		//Matrix - indice - metodo - material
		Geometries[0].Draw(Shader[1]);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		display_fps(window);
		//glfwSwapBuffers(window);
		glFlush();
		glfwPollEvents();

		
	}
	//Delete Buffers
	// optional: de-allocate all resources once they've outlived their purpose:
	//End Program
	glfwTerminate();
	return 0;
}

// Functions
// ************************************************************************************************
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	std::cout << "Resized to " << width << "X" << height << std::endl;
}
void drawGrid()
{
}

void display_fps(GLFWwindow* window)
{
	float fps = ceil(1.0 / deltaTime);
	int ifps = (int)fps;
	std::string titleInfo, titteVersion, fpsCount;
	fpsCount = "FPS: " + std::to_string(ifps);
	titleInfo = APP_TITLE;
	titteVersion = APP_VERSION;
	std::string title = titleInfo +titteVersion + fpsCount;
	glfwSetWindowTitle(window, title.c_str());
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
	// Keyboard Events
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		CurrentCamera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		CurrentCamera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		CurrentCamera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		CurrentCamera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		CurrentCamera.ProcessKeyboard(UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		CurrentCamera.ProcessKeyboard(DOWN, deltaTime);
}
void mouse_callback(GLFWwindow* window, double location_X, double location_Y)
{
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

	CurrentCamera.ProcessMouseMovement(Move_X, Move_Y);
}
void scroll_callback(GLFWwindow* window, double offset_X, double offset_Y)
{
	CurrentCamera.ProcessMouseScroll((float)offset_Y);
}