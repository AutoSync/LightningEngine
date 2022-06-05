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
	LinearColor Background;
	Camera* camera;
	const char* v_path;
	const char* f_path;
	
	GLuint VertexArrayID, vertexbuffer;

public:
	//Constructor
	EditorEngine()
	{
		//Initialize Color
		Background = LinearColor(C3(125, 125, 125), 1.0f);
		//Set path to vertex shader
		v_path = "Shaders/v_test.glsl";
		//Set path to fragment shader
		f_path = "Shaders/f_test.glsl";
		//Create shader
		shader = new Shader(v_path, f_path);
		//Set Camera to default position
		camera = new Camera(V3(4, 3, 3));

		
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
		static const GLfloat g_vertex_buffer_data[] = {
		   -1.0f, -1.0f, 0.0f,
		   1.0f, -1.0f, 0.0f,
		   0.0f,  1.0f, 0.0f,
		};

		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);

		// Generate 1 buffer, put the resulting identifier in vertexbuffer
		glGenBuffers(1, &vertexbuffer);
		// The following commands will talk about our 'vertexbuffer' buffer
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		// Give our vertices to OpenGL.
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
		
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);
		// Draw the triangle !

	}
	//Render Loop
	void Update()
	{
		SetClearColor(Background, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shader->Init();
		
		glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
	}
	void End()
	{
		glDisableVertexAttribArray(0);

	}

};

// Program entry point and execution
int main(int argc, const char* argv[])
{
	EditorEngine* editor = new EditorEngine();
	editor->Init();
	
}
