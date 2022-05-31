#include "Engine.h"

#include "System.h"
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
	uint texture1, texture2;
	Camera* MainCamera;
	Spectator* spec;
	LinearColor Background;
	MeshComponent* object;
public:
	//Constructor
	EditorEngine()
	{
		Background = LinearColor(C3(211, 231, 246), 1.0f);
		shader = new Shader("Shaders/vertexshader.glsl", "Shaders/fragmentshader.glsl");
		object = new MeshComponent("Samples/Primitives/cube.obj");
		MainCamera = new Camera;
		MainCamera->SetLocation(0.0f, 0.0f, -3.0f);
		spec = new Spectator(MainCamera);	
	}
	// Init Engine
	void Init()
	{
		InitializeEngine();
	}

private:
	//When Start Program
	void Start()
	{
		float vertices[] = {
			// positions          // texture coords
			 0.5f,  0.5f, 0.0f,   1.0f, 1.0f, // top right
			 0.5f, -0.5f, 0.0f,   1.0f, 0.0f, // bottom right
			-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, // bottom left
			-0.5f,  0.5f, 0.0f,   0.0f, 1.0f  // top left 
		};
		unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
		};

		unsigned int VBO, VAO, EBO;
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// texture coord attribute
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}
	//Render Loop
	void Update()
	{
		SetClearColor(Background);
		RenderCommand(LR_CLEAR, GL_COLOR_BUFFER_BIT);

		spec->AddInputMovement(Time->deltaTime);
		//spec->GetCamera()->Render();
		projection = glm::perspective(glm::radians(45.0f), (float)840 / (float)480, 0.1f, 100.0f);
		view = spec->GetCamera()->GetViewMatrix();

		shader->Render();
		shader->SetMat4("projection", projection);
		shader->SetMat4("view", view);

		//shader->Model();
		//object->Draw(shader);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	}
	//End program
	void End()
	{
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
	}
};

// Program entry point and execution
int main(int argc, const char* argv[])
{
	EditorEngine* editor = new EditorEngine();
	editor->Init();
}
