#pragma once
#include "lightning-initialize.h"
#include "game-aiplane.h"


#include <vector>

using namespace std;
using namespace Equinox;
using namespace Random;
using namespace Lightning::Stream;
using namespace Lightning::Model;
using namespace Lightning::Modules;
using namespace Lightning::Front;

int Lightning::Model::HardMesh::countmodel = 0;

void CameraControler(GLFWwindow* window);
void airplaneController(GAirplane* airplane);
void ApplicationControl(Shader* shader);
auto camera = new Camera;
auto cah = new CameraHandler(camera);

bool boxvisible = true;
Console console;

int main(int argc, char* argv[])
{
	console.init();
	console.Log("LIGHTNING ENGINE 0.13.0");
	/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
	/* APP CONTEXT INICIALIZATION */
	console.Log("LIGHTNING ENGINE -- LOADING RESOURCE");
	Window App;
	App.init(window, context, framebuffer);
	RenderCommand(LR_DEPTH, LDEFAULT);
	RenderCommand(LR_CULL, LDEFAULT);
	/*APP OBJECTS IMPORT */
	auto importer = new Importer;
	auto eqnxHandler = new Equinox::Handler;
	auto ld = new Lights(Lightning::Front::ELightMode::LIGHT_DIRECT);
	auto fog = new Fog(0.1f, 10.0f);
	vector<Lights>  lp = { Lightning::Front::ELightMode::LIGHT_POINT,
						  Lightning::Front::ELightMode::LIGHT_POINT,
						  Lightning::Front::ELightMode::LIGHT_POINT,
						  Lightning::Front::ELightMode::LIGHT_POINT,
						  Lightning::Front::ELightMode::LIGHT_POINT,
						  Lightning::Front::ELightMode::LIGHT_POINT };

	/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
	/* APP DATA INITIALIZE */
	c4 background = c4(200);
	auto lshader = new Equinox::Shader("vertphong.vert", "fragphong.frag");
	
	fog->setColorFog(c3(212, 210, 190));
	fog->setStartFog(0.0f);
	fog->setEndFog(25.0f);

	c3 red = c3(255, 0, 0);
	c4 color = c4(255, 0, 0, 125);
	vector<c3> lightColors = { c3(255, 0, 0), c3(0,255,0), c3(0,0,255),  c3(255,0,255),
							   c3(255,255,0), c3(0,255,255) };

	auto LegacyShader = new Equinox::Shader("vertphong.vert", "fragphong.frag");
	auto LightShader = new Equinox::Shader("vertlight.vert", "fraglight.frag");
	
	//auto lebox = new HardMesh("Samples/objects/LEBOX.obj");
	//auto box = new HardMesh("Samples/objects/box.obj");
	//auto plane = new HardMesh("Samples/objects/plane.obj");
	auto floor = new HardMesh("Samples/objects/floor.obj");
	//auto sphere = new HardMesh("Samples/objects/floor.obj");
	//auto teapot = new HardMesh("Samples/objects/sphere.obj");
	//auto torus = new HardMesh("Samples/objects/torus.obj");
	//auto torusknot = new HardMesh("Samples/objects/torusknot.obj");
	auto airmesh = new HardMesh("Samples/Templates/airplane/airplane.obj");
	auto airplane = new GAirplane(airmesh, LegacyShader, camera);
	
	

	bool fileshow = true;
	bool style = false;
	bool landcapeDashboard = false;
	/*----------------------------------------------------------------------------*/
	console.Log("LIGHTNING ENGINE -- ENGINE STARTED");
	while (App.getClose())
	{
		Time.setTime(glfwGetTime());										// <- Define o Tempo do Aplicativo												// <- Define o DeltaTime do programa
		mouse = App.getMouseEvents();
		//setInfoTitleBar(false, true);										// <- Define quais informa��es aparecem
		//showInfoTitleBar(App.getWindow(), window, mouse);					//<- Mostra informa��es na barra de titulo
		/*************** PROCESS INPUTS ****************/
		CameraControler(App.getWindow());

		glClearColor(background.r, background.g, background.b, 1.0);		// <- Limpa a tela com uma cor
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//
		/******************** OBJECTS RENDER ********************************* */
		
		projection = glm::perspective(glm::radians(camera->getFOV()),
			(float)window.width / (float)window.height,
			0.1f, 100.0f);
		view = camera->GetViewMatrix();
		LightShader->use();
		LightShader->setMat4("projection", projection);
		LightShader->setMat4("view", view);

		LegacyShader->use();
		ApplicationControl(LegacyShader);
		LegacyShader->setMat4("projection", projection);
		LegacyShader->setMat4("view", view);

		fog->use(LegacyShader);

		LegacyShader->setVec3("ViewLocation", camera->Location);
		//ld->useLight(LegacyShader);
		LegacyShader->setFloat("material.shininess", 64.0f);
		ld->setLightColor(c3(241, 218, 164));
		ld->setAmbientColor(c3(70, 76, 88));
		ld->useLight(LegacyShader);
		
		floor->setLocation(v3(-2.0f, 0.0f, 0.0f));
		floor->Draw(LegacyShader);

		airplaneController(airplane);
		airplane->Render();
		

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		{
			
			if (ImGui::BeginMainMenuBar())
			{
				if (ImGui::BeginMenu("FILE"))
				{
					if (ImGui::BeginMenu("New"))
					{
						ImGui::MenuItem("File", "Ctrl+N");
						ImGui::MenuItem("Project", "Ctrl+Shift+N");
						ImGui::MenuItem("Continuos-Flow", "Ctrl+Alt+N");
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("Open"))
					{
						ImGui::MenuItem("File", "Ctrl+O");
						ImGui::MenuItem("Project", "Ctrl+Shift+O" );
						ImGui::MenuItem("Continuos-Flow", "Ctrl+Alt+O");
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("Save"))
					{
						ImGui::MenuItem("Save Current file","Ctrl+S");
						ImGui::MenuItem("Save As", "Ctrl+Shift+S");
						ImGui::MenuItem("Save All", "Ctrl+Alt+S");
						ImGui::EndMenu();
					}
					ImGui::Separator();
					ImGui::MenuItem("Import File", "CTRL+I");
					ImGui::MenuItem("Export File", "CTRL+E");
					ImGui::Separator();

					if (ImGui::BeginMenu("Package project"))
					{
						ImGui::MenuItem("Windows Binary");
						ImGui::EndMenu();
					}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("EDIT"))
				{
					ImGui::MenuItem("Undo", "CTRL+Z");
					ImGui::MenuItem("Redo", "CTRL+SHIFT+Z");
					ImGui::Separator();
					ImGui::MenuItem("Select-all");
					ImGui::MenuItem("Copy");
					ImGui::MenuItem("Paste");
					ImGui::MenuItem("Cut");
					ImGui::Separator();
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("TOOLS"))
				{
					if (ImGui::BeginMenu("Add Geometry"))
					{
						ImGui::MenuItem("Box");
						ImGui::MenuItem("Sphere");
						ImGui::MenuItem("Plane");
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("Landscape Editor"))
					{
						ImGui::MenuItem("Create Landscape");
						ImGui::Checkbox("Open Landscape Editor Dashboard", &landcapeDashboard);
						ImGui::EndMenu();
					}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("WINDOW"))
				{
					ImGui::MenuItem("Editor");
					ImGui::MenuItem("Customize");
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("HELP"))
				{
					ImGui::MenuItem("About us");
					ImGui::EndMenu();
				}
				
			ImGui::EndMainMenuBar();
			}
			
		}

		App.renderGUI();
		App.broadcast();											//<- Processa o manipulador de eventos 
	}

	App.release();													// <- Encerra o contexto e limpa os buffers

	return 0;
}

void CameraControler(GLFWwindow* window)
{
	// Set
	auto input = new Input;
	if (input->getKeyPress(U))
		isProjection = false;
	if (input->getKeyPress(P))
		isProjection = true;
	if (input->getMouseButton(MOUSE_RIGHT))
	{
		input->setShowCursor(false);
		if (input->getKeyPress(W))
			camera->SetInputMovement(EVectorMovement::FORWARD, (float)Time.deltaTime);
		if (input->getKeyPress(S))
			camera->SetInputMovement(EVectorMovement::BACKWARD, (float)Time.deltaTime);
		if (input->getKeyPress(A))
			camera->SetInputMovement(EVectorMovement::LEFT, (float)Time.deltaTime);
		if (input->getKeyPress(D))
			camera->SetInputMovement(EVectorMovement::RIGHT, (float)Time.deltaTime);
		if (input->getKeyPress(Q))
			camera->SetInputMovement(EVectorMovement::DOWN, (float)Time.deltaTime);
		if (input->getKeyPress(E))
			camera->SetInputMovement(EVectorMovement::UP, (float)Time.deltaTime);
	}
	else
		input->setShowCursor(true);
	double Move_X, Move_Y;

	if (fistMouse)
	{
		lastX = mouse.position.x;
		lastY = mouse.position.y;
		fistMouse = false;
	}

	Move_X = mouse.position.x - lastX;
	Move_Y =  lastY - mouse.position.y;
	lastX = mouse.position.x;
	lastY = mouse.position.y;

	if (input->getMouseButton(MOUSE_RIGHT))
	{
		input->setShowCursor(false);
		camera->setInputYaw(Move_X);
		camera->setInputPitch(Move_Y);
		if (input->getKeyPress(F))
			camera->setInputZoom(10, (float)Time.deltaTime);
		if (input->getKeyPress(V))
			camera->setInputZoom(-10, (float)Time.deltaTime);
	}
	else
		input->setShowCursor(true);
}

void airplaneController(GAirplane* airplane)
{
	auto input = new Input;
	if (input->getKeyPress(W))
		airplane->throtle(1 * (float)Time.deltaTime);
	if (input->getKeyPress(S))
		airplane->throtle(-1 * (float)Time.deltaTime);
	if (input->getKeyPress(A))
		airplane->steer(-1 * (float)Time.deltaTime);
	if (input->getKeyPress(D))
		airplane->steer(1 * (float)Time.deltaTime);
	if (input->getKeyPress(Q))
		airplane->HeightControl(-1 * (float)Time.deltaTime);
	if (input->getKeyPress(E))
		airplane->HeightControl(1 * (float)Time.deltaTime);
}

void ApplicationControl(Shader* shader)
{
	auto input = new Input;
	/*Type of Shading*/
	if (input->getKeyPress(KEY_1))
		setShading(shader,"typeShading", 0);
	if (input->getKeyPress(KEY_2))
		setShading(shader, "typeShading", 1);
	if (input->getKeyPress(KEY_3))
		setShading(shader, "typeShading", 2);
	if (input->getKeyPress(KEY_4))
		setShading(shader, "typeShading", 3);
}