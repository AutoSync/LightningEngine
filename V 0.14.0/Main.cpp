#include "Engine.h"

#include "Types.h"
#include "Shader.h"
#include "Geometry.h"
#include "Camera.h"
#include "Lights.h"

//STD
#include <iostream>
using namespace Lightning;

class EditorEngine : Engine
{
	uint VAO = 0, VBO = 0;
	Shader* shader = new Shader("Shaders/vertexshader.glsl", "Shaders/fragmentshader.glsl");
	glm::mat4 model, view, projection;
private:

public:
	//Constructor
	EditorEngine()
	{
		
	}
	// Init Engine
	void Init()
	{
		InitializeEngine();
	}
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

};

// Program entry point and execution
int main(int argc, const char* argv[])
{
	auto editor = new EditorEngine();
	editor->Init();
}

