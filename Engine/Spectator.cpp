#include "Spectator.h"

Lightning::Spectator::Spectator(Camera& Camera)
{
	Input = new Inputs(glfwGetCurrentContext());
	camera = &Camera;
}

Camera* Lightning::Spectator::GetCamera()
{
	return camera;
}

void Lightning::Spectator::AddInputMovement()
{
	Expected(Time->deltaTime == 0.0f, "delta time cannot be null, try a manual approach");
	double Move_X, Move_Y, delta = Time->deltaTime;
	
	
	if (Input->GetMousePress(MouseKeys::MOUSE_RIGHT))
	{
		Input->SetHideCursor(true);

		if (Input->GetKeyPress(Keyboard::W))
			camera->SetInputMovement(Direction::FORWARD, delta);
		if (Input->GetKeyPress(Keyboard::S))
			camera->SetInputMovement(Direction::BACKWARD, delta);
		if (Input->GetKeyPress(Keyboard::A))
			camera->SetInputMovement(Direction::LEFT, delta);
		if (Input->GetKeyPress(Keyboard::D))
			camera->SetInputMovement(Direction::RIGHT, delta);
		if (Input->GetKeyPress(Keyboard::Q))
			camera->SetInputMovement(Direction::DOWN, delta);
		if (Input->GetKeyPress(Keyboard::E))
			camera->SetInputMovement(Direction::UP, delta);
		
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

		Msg::Emit(Flow::PRINT, "Delta Time: " + std::to_string(delta));
	}
	else
		Input->SetHideCursor();
}
