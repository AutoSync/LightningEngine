#include "Spectator.h"

Lightning::Spectator::Spectator(Camera* cam)
{
	camera = cam;
}

void Lightning::Spectator::CameraMovement(float dt)
{
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

		double Move_X, Move_Y;

		lastX = mouse.position.x;
		lastY = mouse.position.y;

		Move_X = mouse.position.x - lastX;
		Move_Y = lastY - mouse.position.y;
		lastX = mouse.position.x;
		lastY = mouse.position.y;

		camera->SetInputYaw(Move_X);
		camera->SetInputPitch(Move_Y);

		/*if (input->getKeyPress(F))
			camera->setInputZoom(10, (float)Time.deltaTime);
		if (input->getKeyPress(V))
			camera->setInputZoom(-10, (float)Time.deltaTime);*/
	}
	else
		Input->SetHideCursor();
}
