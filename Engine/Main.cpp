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
	Shader* shader;
	Camera* MainCamera;
	Spectator* spec;
	LinearColor Background;
	MeshComponent* object;
	Light* light;
public:
	//Constructor
	EditorEngine()
	{
		Background = LinearColor(C3(211, 231, 246), 1.0f);
		shader = new Shader("Shaders/vertexshader.glsl", "Shaders/fragmentshader.glsl");
		object = new MeshComponent("Samples/Primitives/cube.obj");
		
		light = new Light(Transform(0.0f), LightType::Directional);
		light->diffuse = C3(245, 245, 211);
		light->ambient = C3(52, 62, 103);
		
		MainCamera = new Camera;
		MainCamera->SetLocation(0.0f, 1.0f, -3.0f);
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
		
	}
	//Render Loop
	void Update()
	{
		float t = (float)Time->GetTime();
		//cout << Mix(1.0, 0.0, sin(t)) << endl;
		LinearColor YellowBackground = LinearColor(C3(245, 245, 211), 1.0f);
		LinearColor NewBackground = Cinterp(Background, YellowBackground, sin(t));
		SetClearColor(NewBackground);

		RenderCommand(LR_CLEAR, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		spec->AddInputMovement(Time->deltaTime);
		
		
		shader->Init();
		
		shader->SetMat4("projection", projection);
		shader->SetMat4("view", view);
	
		light->Render(shader);
		shader->SetV3("ViewLocation", spec->GetCamera()->GetLocation());
		
		object->Draw(shader);
		object->SetScale(V3(0.5f));
	}

};

// Program entry point and execution
int main(int argc, const char* argv[])
{
	EditorEngine* editor = new EditorEngine();
	editor->Init();
}
