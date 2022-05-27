#include <windows.h>
#include "SystemStart.h"

//Prints stack of events with elapsed time
// ************************************************************************************************														//Set Delta Time
void setTime();
float getDeltaTimeSeconds();
void setDeltaTime();												
void display_fps(GLFWwindow* window);											//Show FPS in Title Bar
void framebuffer_size_callback(GLFWwindow* window, int width, int height);		//Window function
void processInput(GLFWwindow* window);											//Key Press
void mouse_callback(GLFWwindow* window, double location_X, double location_Y);	//Mouse Input
void scroll_callback(GLFWwindow* window, double offset_X, double offset_Y);		//Scroll Input
//#####################################################################################
float ASPECT_RATIO = 1.77;
int SCR_X = 1024;																//X-axis resolution
int SCR_Y = SCR_X / ASPECT_RATIO;												//Y-axis resolution
string APP_VERSION = "0.12.0";													//Version App
string APP_TITLE = "LIGHTNING ENGINE " + APP_VERSION;							//Title App

Print Console;																		//Print Screen
CameraObject FirstCamera(glm::vec3(0.0f,1.0f,3.0f));							//Camera placement
CameraController CurrentCamera(FirstCamera);									//Camera Controller
float Time, deltaTime, lastFrame;												//Time Game
float lastX = SCR_X / 2;														//Mouse Location in X
float lastY = SCR_Y / 2;														//Mouse Location in Y
bool fistMouse = true;
int viewMode = 0;																//Pespective View
float nearClip = 0.1f;	
float farClip = 20.0f;
bool isMoving = false;															//Enable render when is moving false
int renderMode = 0;																//0 = Lit 
int postfx = 0;

int main(int argc, char* argv[])
{
	char* path = argv[0];

	Console.init();
	cout <<"2020 (C) AutoSync " << APP_TITLE << endl;
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);
	#ifdef __APPLE__															//Apple Mac compatibility
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	#endif

	GLFWwindow* window = glfwCreateWindow(SCR_X, SCR_Y, APP_TITLE.c_str(), NULL, NULL);
	if (window == NULL)
	{
		Console.now("ERROR :( FAILED TO CREATE GLFW WINDOW");
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
		Console.now("ERROR :( FAILED TO INITIALIZE GLAD");
		return -1;
	}
	Console.Log("ENGINE STARTED");
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	//glDepthFunc(GL_LESS);													
	//glEnable(GL_STENCIL_TEST);
	//glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	//glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	//POST PROCESS



	Console.Log("GL FUNCTIONS");
	//stbi_set_flip_vertically_on_load(true);									
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//Objects
	Console.Log("LIGHT SCENE BUILD");
	LightDirectional sun;
	sun.setDiffuse(1.0f);
	sun.setAmbient(0.79f, 0.82f, 0.95f);
	sun.setSpecular(0.25);
	sun.setLightDirection(-0.2f, -1.0f, -0.3f);
	glm::vec4 Environment = glm::vec4(sun.getAmbient(), 1.0);

	Shader shader("EngineContents/Shaders/MaterialSample.vert", "EngineContents/Shaders/MaterialSample.frag");
	Shader outliner("EngineContents/Shaders/MaterialSample.vert", "EngineContents/Shaders/ObjectOutline.frag");
	Shader sshader("EngineContents/Shaders/ScreenShader.vert", "EngineContents/Shaders/ScreenShader.frag");
	Console.Log("SHADERS STARTED");
	
	Model Background("EngineContents/Meshes/Sampler/bkPlane.obj");
	Model LBOX("EngineContents/Meshes/Sampler/LE_Box.obj");
	/*Model Sphere("EngineContents/Meshes/Sampler/Sphere.obj");
	Model Box("EngineContents/Meshes/Sampler/Box.obj");
	Model BS("EngineContents/Meshes/Sampler/BoxNoise.obj");
	Model Plant("EngineContents/Meshes/Sampler/PlantSample.obj");
	Model LE("EngineContents/Meshes/Sampler/BlendSample.obj");*/

	Model b0 = LBOX;
	Model b1 = LBOX;
	Model b2 = LBOX;
	Model b3 = LBOX;
	Model b4 = LBOX;
	vector<Model> boxes
	{b0, b1, b2, b3, b4};
	vector<glm::vec3> boxPositions
	{
		glm::vec3(-2.5f, 0.0f, -2.5f),
		glm::vec3(-1.0f, 0.0f, -2.5f),
		glm::vec3(0.0f, 0.0f, 1.5f),
		glm::vec3(1.0f, 0.0f, 2.0f),
		glm::vec3(2.5f, 0.0f, 2.5f)
	};
	Console.Log("MODELS LOADED");
	
	sshader.use();
	sshader.setInt("screenTexture", 0);

	float quadVertices[] = {
		// vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};

	unsigned int quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	//GL++PARAMETERS

	//FRAMEBUFFERS
	unsigned int framebuffer, textureColor, renderBuffer;

	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	/*unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, screen_width, screen_height, 0,
				GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture, 0);*/

	glGenTextures(1, &textureColor);
	glBindTexture(GL_TEXTURE_2D, textureColor);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_X, SCR_Y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColor, 0);

	glGenRenderbuffers(1, &renderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_X, SCR_Y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		Console.now("ERROR :( FRAMEBUFFER NOT COMPLETE");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	Console.Log("FRAMEBUFFERS CREATED");
	// Pespectiva 
	Console.Log("RENDERING STAGE");
	while (!glfwWindowShouldClose(window))									
	{
		//Set Delta Time
		setTime();
		setDeltaTime();
		// Input Keyboard/Mouse
		processInput(window);
		display_fps(window);
		//Render Loop
		
		//FRAMEBUFFER STATE
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glEnable(GL_DEPTH_TEST);
		//
		glm::mat4 projection = glm::mat4(1.0f);
		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 model = glm::mat4(1.0f);
		switch (viewMode)
		{
		case 0:
			projection = glm::perspective(glm::radians(CurrentCamera.ActiveCamera.Zoom), (float)SCR_X / (float)SCR_Y, nearClip, farClip);
			view = CurrentCamera.ActiveCamera.GetViewMatrix();
			break;
		case 1:
			projection = glm::ortho(0.0f, (float)SCR_X, 0.0f, (float)SCR_Y, 0.0f, 1.0f);
			view = CurrentCamera.ActiveCamera.GetOrthoMatrix();
		default:
			projection = glm::perspective(glm::radians(CurrentCamera.ActiveCamera.Zoom), (float)SCR_X / (float)SCR_Y, nearClip, farClip);
			view = CurrentCamera.ActiveCamera.GetViewMatrix();
			break;
		}
		switch (renderMode)
		{
		case 0:
			glClearColor(Environment.x, Environment.y, Environment.z, Environment.w);
			//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			break;
		case 1: 
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			break;
		case 2:
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			break;
		case 3:
			glClearColor(Environment.x, Environment.y, Environment.z, Environment.w);
			//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			break;
		case 4:
			glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
			//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			break;
		default:
			glClearColor(Environment.x, Environment.y, Environment.z, Environment.w);
			//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			break;
		}
		
		outliner.use();
		outliner.setMat4("projection", projection);
		outliner.setMat4("view", view);
	
		shader.use();
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);
		shader.setInt("renderMode", renderMode);

		shader.setVec3("ViewLocation", CurrentCamera.ActiveCamera.Location);
		shader.setVec3("ldir.direction", sun.getLightDirection());
		shader.setVec3("ldir.diffuse", sun.getDiffuse());
		shader.setVec3("ldir.specular", sun.getSpecular());
		shader.setVec3("ldir.ambient", sun.getAmbient()* 0.2f);
		shader.setFloat("material.shininess", 64.0f);
		

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f));
		shader.setMat4("model", model);
		Background.Draw(shader);
		
		for (unsigned int i = 0; i < boxPositions.size(); i++)
		{
			//model = glm::mat4(0.0f);
			model = glm::translate(model,boxPositions[i]);
			shader.setMat4("model", model);
			boxes[i].Draw(shader);
		}
		//POST PROCESSING
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		sshader.use();
		sshader.setInt("postfx", postfx);

		glBindVertexArray(quadVAO);
		glBindTexture(GL_TEXTURE_2D, textureColor);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(window);
		glFlush();
		glfwPollEvents();

	}
	glfwTerminate();
	return 0;
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	SCR_X = width;
	SCR_Y = width / ASPECT_RATIO;
	glViewport(0, 0, SCR_X, SCR_Y);
	std::cout << "Resized to " << width << "X" << height << std::endl;
}
void setTime()
{
	Time = (float)glfwGetTime();
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

void display_fps(GLFWwindow* window)
{
	string title = APP_TITLE + " FPS: " + to_string(int(floor(1.0 / deltaTime))) + 
		" DeltaTime: " + to_string(deltaTime) + " FOV: " + to_string(int(CurrentCamera.ActiveCamera.Zoom));
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

	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS && viewMode != 1)
	{
		viewMode = 1; // ORTHO
		
		Console.Log("ORTHO MODE");
	}
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && viewMode != 0)
	{
		viewMode = 0; // PESPECTIVE
		Console.Log("PESPECTIVE MODE");
	}
	//Lit Mode
	if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS)
	{
		renderMode = 0;
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	}
	//Unlit mode
	if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS)
	{
		renderMode = 1;
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	//Solid Shader
	if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS)
	{
		renderMode = 2;
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	//Gamemode
	if (glfwGetKey(window, GLFW_KEY_F4) == GLFW_PRESS)
	{
		renderMode = 3;
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	//Wireframe
	if (glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS)
	{
		renderMode = 4;
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	// Efeitos de pós processar
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
	{
		postfx = 1; // DEFAULT
	}
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
	{
		postfx = 2; // INVERT
	}
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
	{
		postfx = 3; // GRAYSCALE
	}
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
	{
		postfx = 4; // TOXIC
	}
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
	{
		postfx = 5; // BLUR
	}
	if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
	{
		postfx = 6; // DETECT EDGE
	}
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
	double Move_X = location_X - lastX;
	double Move_Y = lastY - location_Y;
	lastX = location_X;
	lastY = location_Y;

	if (mouse_right == GLFW_PRESS)
		CurrentCamera.ActiveCamera.ProcessMouseMovement(Move_X, Move_Y);
}

void scroll_callback(GLFWwindow* window, double offset_X, double offset_Y)
{
	CurrentCamera.ActiveCamera.ProcessMouseScroll(offset_Y);
}