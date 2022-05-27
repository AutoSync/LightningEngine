#include "Camera.h"

Lightning::Camera::Camera()
{
	Transform T = Transform();
	InitializeCamera(T);
}

Lightning::Camera::Camera(Transform T)
{
	InitializeCamera(T);
}

void Lightning::Camera::Render()
{
	view = glm::lookAt(transform.Position.glm, transform.Position.glm + front.glm, CameraUp);
}

void Lightning::Camera::SetPosition(V3 position)
{
	this->transform.Position = position;
}

void Lightning::Camera::SetRotation(V3 rotation)
{
	this->transform.Rotation = rotation;
}

void Lightning::Camera::AddInput(float dt)
{
	if (Active)
	{
		if (Input->GetMousePress(MouseKeys::MOUSE_RIGHT))
		//if(true)
		{
			Input->SetHideCursor(true);
			if (Input->GetKeyPress(Keyboard::W))
			{
				transform.Position.x += MovementSpeed * dt;
			}
			if (Input->GetKeyPress(Keyboard::S))
			{
				transform.Position.x -= MovementSpeed * dt;
			}
			if (Input->GetKeyPress(Keyboard::A))
			{
				transform.Position.z -= MovementSpeed * dt;
			}
			if (Input->GetKeyPress(Keyboard::D))
			{
				transform.Position.z += MovementSpeed * dt;
			}
			if (Input->GetKeyPress(Keyboard::Q))
			{
				transform.Position.y -= MovementSpeed * dt;
			}
			if (Input->GetKeyPress(Keyboard::E))
			{
				transform.Position.y += MovementSpeed * dt;
			}
			
		}
		else
		{
			Input->SetHideCursor(false);
		}

	}
}

glm::mat4 Lightning::Camera::GetViewMatrix()
{
	return glm::lookAt(transform.Position.glm, transform.Position.glm + front.glm, Up.glm);
}

void Lightning::Camera::SetComponentActive(bool B)
{
	this->Active = B;
}


inline void Lightning::Camera::InitializeCamera(Transform T)
{
	this->transform = T;
}

void Lightning::Camera::UpdateCameraVectors()
{
	front = MakeRotate(glm::vec3(Rotation.x, Rotation.y, Rotation.z));
	// Also re-calculate the Right and Up vector
	right = glm::normalize(glm::cross(front.glm, worldUp.glm));
	Up = glm::normalize(glm::cross(right, front.glm));
}

