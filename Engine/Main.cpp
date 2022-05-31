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
		object = new MeshComponent("Samples/Primitives/cube.obj");
		MainCamera = new Camera;
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
		RenderCommand(LR_CLEAR, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		spec->AddInputMovement(Time->deltaTime);

		spec->GetCamera()->SetProjection(projection);
		spec->GetCamera()->SetView(view);

		shader->Projection(projection);
		shader->View(view);



	}
	//End program
	void End()
	{

	}
};

// Program entry point and execution
int main(int argc, const char* argv[])
{
	EditorEngine* editor = new EditorEngine();
	editor->Init();
}
