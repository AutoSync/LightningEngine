
#include "Engine.h"
using namespace Lightning;

class EditorEngine : Engine
{
public:

	// Variables of Program
	uint counting = 0;
	LinearColor color = LinearColor(0.0, 0.0, 0.0, 1.0);
	
	EditorEngine()
	{
		InitializeEngine();
		SetShowFramerate(true);

		

	}

	void Update()
	{
		counting++;
		glClearColor(color.r, color.g, color.b, color.a);
		
		float intensity = 0.2f;
		if (Input->GetKeyPress(GLFW_KEY_ESCAPE))
			ExitProgram();
		// Changing Color Red
		if (Input->GetKeyPress(GLFW_KEY_KP_4))
		{
			color.r = IncrementRangef('+', color.r, intensity * Time->deltaTime);
			cout << "VERMELHO: " << color.r << '\n';
		}
		if (Input->GetKeyPress(GLFW_KEY_KP_1))
		{
			color.r = IncrementRangef('-', color.r, intensity * Time->deltaTime);
			cout << "VERMELHO: " << color.r << '\n';
		}
		// Changing Color Green
		if (Input->GetKeyPress(GLFW_KEY_KP_5))
		{
			color.g = IncrementRangef('+', color.g, intensity * Time->deltaTime);
			cout << "VERDE: " << color.g << '\n';
		}
		if (Input->GetKeyPress(GLFW_KEY_KP_2))
		{
			color.g = IncrementRangef('-', color.g, intensity * Time->deltaTime);
			cout << "VERDE: " << color.g << '\n';
		}
		// Changing Color Blue
		if (Input->GetKeyPress(GLFW_KEY_KP_6))
		{
			color.b = IncrementRangef('+', color.b, intensity * Time->deltaTime) ;
			cout << "AZUL: " << color.b << '\n';
		}
		if (Input->GetKeyPress(GLFW_KEY_KP_3))
		{
			color.b = IncrementRangef('-', color.b, intensity * Time->deltaTime);
			cout << "AZUL: " << color.b << '\n';
		}


	}
};

// Program entry point and execution
int main(int argc, const char* argv[])
{
	auto editor = new EditorEngine();
}

