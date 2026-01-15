#include "Engine.h"

#include "Types.h"
#include "Shader.h"
#include "Geometry.h"
#include "Camera.h"
#include "Lights.h"
#include "Image.h"
#include "Spectator.h"

using namespace Lightning;

class EditorEngine : Engine
{
private:
	Image img_import;
	uint VAO, VBO, EBO;
	Shader* shader;
	glm::mat4 model, view, projection;
	uint texture1, texture2;
	Camera* MainCamera;
	Spectator* spc;
	vector<MeshComponent*> Meshes;

public:
	//Constructor
	EditorEngine()
	{
		shader = new Shader("Shaders/vertexshader.glsl", "Shaders/fragmentshader.glsl");
		texture1 = img_import.TextureFromFile("Samples/texture/box.jpg");
		texture2 = img_import.TextureFromFile("Samples/texture/le.png");

		MeshComponent* mesh1 = new MeshComponent("Samples/Primitives/box.obj");
		MeshComponent* mesh2 = new MeshComponent("Samples/Primitives/plane.obj");
		Meshes.push_back(mesh1);
		Meshes.push_back(mesh2);

		MainCamera = new Camera();
		spc = new Spectator(MainCamera);
		
	}
	// Init Engine
	void Init()
	{
		InitializeEngine();
	}
private:
	void Start()
	{
		
	}
	//Render Loop
	void Update()
	{
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// activate shader
		shader->Render();

		spc->CameraMovement( Time->deltaTime);
		// create transformations
		model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
		view = MainCamera->GetViewMatrix();
		projection = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
		projection = glm::perspective(glm::radians(45.0f), (float)engine_settings.width / (float)engine_settings.height, 0.1f, 100.0f);
		// retrieve the matrix uniform locations
		shader->SetMat4("model", model);
		shader->SetMat4("view", view);
		// note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
		shader->SetMat4("projection", projection);

		for (int i = 0; i < Meshes.size(); i++)
		{
			model = glm::mat4(1.0f);
			shader->SetMat4("model", model);
			Meshes[i]->Draw(shader);
		}


	
	}
	//End program
	void End()
	{
		
	}
};

// Program entry point and execution
int main(int argc, const char* argv[])
{
	auto editor = new EditorEngine();
	editor->Init();
}
