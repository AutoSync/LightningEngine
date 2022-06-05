#include "Engine.h"

#include "Types.h"
#include "Shader.h"
#include "Geometry.h"
#include "Camera.h"
#include "Lights.h"
#include "Image.h"
#include "Spectator.h"
#include "System.h"

using namespace Lightning;

void SizeCallback(GLFWwindow* wnd, int width, int height)
{
	Settings.width = width;
	Settings.height = height;
	glViewport(0, 0, width, height);
	Msg::Emit(Flow::OUTPUT, "Window size changed to: " + to_string(width) + " X " + to_string(height));
}

class EditorEngine : Engine
{
private:
	Shader* shader;
	LinearColor Background;
	Camera* camera;
	Spectator *spec;
	const char* v_path;
	const char* f_path;
	
	GLuint VertexArrayID, vertexbuffer;

public:
	//Constructor
	EditorEngine()
	{
		glfwSetWindowSizeCallback(glfwGetCurrentContext(), SizeCallback);
		//Initialize Color
		Background = LinearColor(C3(125, 125, 125), 1.0f);
		//Set path to vertex shader
		v_path = "Shaders/v_test.glsl";
		//Set path to fragment shader
		f_path = "Shaders/f_test.glsl";
		//Create shader
		shader = new Shader(v_path, f_path);
		//Set Camera to default position
		camera = new Camera();
		camera->SetPosition(4.0f, 3.0f, 3.0f);
		//Set Spectator to default position
		spec = new Spectator(camera);

		
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
		
		spec->AddInputMovement(Time->deltaTime);
		
		projection = spec->GetCamera()->GetPespective();
		view = spec->GetCamera()->GetViewMatrix();
		model = glm::mat4(1.0f);
		
		glm::mat4 MVP = projection * view * model;
		shader->SetMat4("MVP", MVP);
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
