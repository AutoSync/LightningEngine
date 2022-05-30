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
		


	}
	//Render Loop
	void Update()
	{
		
	}
	//End program
	void End()
	{

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
