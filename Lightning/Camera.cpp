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

void Lightning::Camera::CameraMovement(V3 position)
{
	this->transform.Position = position;
}

void Lightning::Camera::CameraRotation(V3 rotation)
{
	this->transform.Rotation = rotation;
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

