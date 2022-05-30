#include "Camera.h"

glm::mat4 Lightning::Camera::GetViewMatrix()
{
    return glm::lookAt(Position.GetGLM(), Position.GetGLM() + Front.GetGLM(), Up.GetGLM());
}
void Lightning::Camera::SetInputMovement(Direction direction, float deltaTime)
{
	float Velocity = MovementSpeed * deltaTime;
	switch (direction)
	{
	case Lightning::FORWARD:
		Position += Front * Velocity;
		break;
	case Lightning::BACKWARD:
		Position -= Front * Velocity;
		break;
	case Lightning::RIGHT:
		Position += Right * Velocity;
		break;
	case Lightning::LEFT:
		Position -= Right * Velocity;
		break;
	case Lightning::UP:
		Position += Up * Velocity;
		break;
	case Lightning::DOWN:
		Position -= Up * Velocity;
		break;
	}
}
void Lightning::Camera::SetRotation(float YAW, float PITCH, float ROLL)
{
	this->Yaw = YAW;
	this->Pitch = PITCH;
	this->Roll = ROLL;
	UpdateCameraVectors();
}
void Lightning::Camera::SetInputYaw(double input)
{
	input *= Sensitivity;
	Yaw += input;
	UpdateCameraVectors();
}
void Lightning::Camera::SetInputPitch(double input, bool ConstraintPitch)
{
	SClamp AngleConstraint(-89.f, 89.f);
	input *= Sensitivity;
	Pitch += input;
	if (ConstraintPitch)
		Pitch = AngleConstraint.clamp(Pitch);
	UpdateCameraVectors();
}
void Lightning::Camera::SetInputZoom(double offset_y, float deltatime)
{
	SClamp depthConstraint(1.0f, 45.0f);
	FOV -= offset_y * deltatime;
	FOV = depthConstraint.clamp(FOV);
}
float Lightning::Camera::GetFOV()
{
	return FOV;
}
void Lightning::Camera::SetFOV(float newFOV)
{
	this->FOV = newFOV;
}
Lightning::V3 Lightning::Camera::GetLocation()
{
	return Position;
}
void Lightning::Camera::SetLocation(float x, float y, float z)
{
	this->Position = V3(x, y, z);
}
void Lightning::Camera::UpdateCameraVectors()
{
	Front = MakeRotate(glm::vec3(Yaw, Pitch, 0.0f));
	// Also re-calculate the Right and Up vector
	Right = glm::normalize(glm::cross(Front.GetGLM(), WorldUp.GetGLM()));
	Up = glm::normalize(glm::cross(Right.GetGLM(), Front.GetGLM()));
}