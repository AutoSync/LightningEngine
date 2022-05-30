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
	Spectator* spec;
	LinearColor Background;
	MeshComponent* object;
public:
	//Constructor
	EditorEngine()
	{
		Background = LinearColor(C3(211, 231, 246), 1.0f);
		shader = new Shader("Shaders/vertexshader.glsl", "Shaders/fragmentshader.glsl");
		object = new MeshComponent("Samples/Primitives/box.obj");
		MainCamera = new Camera();
		spec = new Spectator(MainCamera);
		
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
		SetClearColor(Background);
		spec->AddInputMovement(Time->deltaTime);
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
