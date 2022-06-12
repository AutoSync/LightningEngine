#include "Spectator.h"

Lightning::Spectator::Spectator(Camera* Camera)
{
	Input = new Inputs(glfwGetCurrentContext());
	camera = Camera;
}

Camera* Lightning::Spectator::GetCamera()
{
	return camera;
}

void Lightning::Spectator::Update()
{
	
	projection = camera->GetPespective();
	view = camera->GetLookAt();
}

void Lightning::Spectator::SetPespective(bool Perspective)
{
	camera->usePerspective = Perspective;
}

void Lightning::Spectator::AddInputMovement()
{
	double Move_X, Move_Y;
	double delta = 0.005;
	//double delta = Time->deltaTime;
	
	if (Input->GetMousePress(MouseKeys::MOUSE_RIGHT))
	{
		Input->SetHideCursor(true);

		if (Input->GetKeyPress(Keyboard::W))
		{
			camera->Position += V3(camera->Position.x, 0, 0) * delta;
			cout << "W " << camera->Position.x << endl;
		}
		if (Input->GetKeyPress(Keyboard::S))
		{
			camera->Position -= V3(camera->Position.x, 0, 0) * delta;
			cout << "S " << camera->Position.x << endl;
		}
		if (Input->GetKeyPress(Keyboard::A))
		{
			camera->Position -= V3(0, 0, camera->Position.z) * delta;
			cout << "A " << camera->Position.z << endl;
		}
		if (Input->GetKeyPress(Keyboard::D))
		{
			camera->Position += V3(0, 0, camera->Position.z) * delta;
			cout << "D " << camera->Position.z << endl;
		}
		if (Input->GetKeyPress(Keyboard::Q))
		{
			camera->Position -= V3(0, camera->Position.y, 0) * delta;
			cout << "Q " << camera->Position.y << endl;
		}
		if (Input->GetKeyPress(Keyboard::E))
		{
			camera->Position += V3(0, camera->Position.y, 0) * delta;
			cout << "E " << camera->Position.y << endl;
		}
		
		if (firstMouse)
		{
			lastX = mouse.position.x;
			lastY = mouse.position.y;
			firstMouse = false;
		}
		Move_X = mouse.position.x - lastX;
		Move_Y = lastY - mouse.position.y;
		lastX = mouse.position.x;
		lastY = mouse.position.y;

		camera->SetInputYaw(Move_X);
		camera->SetInputPitch(Move_Y);

		//Msg::Emit(Flow::PRINT, "Delta Time: " + std::to_string(delta));
	}
	else
		Input->SetHideCursor();
}
