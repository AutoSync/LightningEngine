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

void Lightning::Spectator::AddInputMovement(float dt)
{
	double Move_X, Move_Y;
	if (Input->GetMousePress(MouseKeys::MOUSE_RIGHT))
	{
		Input->SetHideCursor(true);

		if (Input->GetKeyPress(Keyboard::W))
			camera->SetInputMovement(Direction::FORWARD, dt);
		if (Input->GetKeyPress(Keyboard::S))
			camera->SetInputMovement(Direction::BACKWARD, dt);
		if (Input->GetKeyPress(Keyboard::A))
			camera->SetInputMovement(Direction::LEFT, dt);
		if (Input->GetKeyPress(Keyboard::D))
			camera->SetInputMovement(Direction::RIGHT, dt);
		if (Input->GetKeyPress(Keyboard::Q))
			camera->SetInputMovement(Direction::DOWN, dt);
		if (Input->GetKeyPress(Keyboard::E))
			camera->SetInputMovement(Direction::UP, dt);
		
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

	}
	else
		Input->SetHideCursor();
}
