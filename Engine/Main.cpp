#include "Engine.h"

#include "Types.h"
#include "Shader.h"
#include "Geometry.h"
#include "Camera.h"
#include "Lights.h"
#include "Image.h"

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

public:
	//Constructor
	EditorEngine()
	{
		shader = new Shader("Shaders/vertexshader.glsl", "Shaders/fragmentshader.glsl");
		texture1 = img_import.TextureFromFile("Samples/texture/box.jpg");
		texture2 = img_import.TextureFromFile("Samples/texture/le.png");

		MainCamera = new Camera();
		
	}
	// Init Engine
	void Init()
	{
		InitializeEngine();
	}
private:
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
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture2);

		// activate shader
		shader->Render();

		CameraMovement(MainCamera, Time->deltaTime);
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

		// render container
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
	void CameraMovement(Camera* camera, float deltaTime)
	{
		if (Input->GetMousePress(MouseKeys::MOUSE_RIGHT))
		{
			if (Input->GetKeyPress(Keyboard::W))
				camera->SetInputMovement(Direction::FORWARD, deltaTime);
			if (Input->GetKeyPress(Keyboard::S))
				camera->SetInputMovement(Direction::BACKWARD, deltaTime);
			if (Input->GetKeyPress(Keyboard::A))
				camera->SetInputMovement(Direction::LEFT, deltaTime);
			if (Input->GetKeyPress(Keyboard::D))
				camera->SetInputMovement(Direction::RIGHT, deltaTime);
			if (Input->GetKeyPress(Keyboard::Q))
				camera->SetInputMovement(Direction::DOWN, deltaTime);
			if (Input->GetKeyPress(Keyboard::E))
				camera->SetInputMovement(Direction::UP, deltaTime);
		}
	}
};

// Program entry point and execution
int main(int argc, const char* argv[])
{
	auto editor = new EditorEngine();
	editor->Init();
}
