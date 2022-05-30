#include "lightning-modules-camera.h"

glm::mat4 Lightning::Modules::Camera::GetViewMatrix()
{
    return glm::lookAt(Location, Location + Front, Up);
}

void Lightning::Modules::Camera::SetInputMovement(EVectorMovement direction, float deltaTime)
{
	using namespace Random::Enums;
	float Velocity = MovementSpeed * deltaTime;
	if (direction == EVectorMovement::FORWARD)
		Location += Front * Velocity;
	if (direction == EVectorMovement::BACKWARD)
		Location -= Front * Velocity;
	if (direction == EVectorMovement::LEFT)
		Location -= Right * Velocity;
	if (direction == EVectorMovement::RIGHT)
		Location += Right * Velocity;
	if (direction == EVectorMovement::UP)
		Location += Up * Velocity;
	if (direction == EVectorMovement::DOWN)
		Location -= Up * Velocity;
}

void Lightning::Modules::Camera::SetRotation(float YAW, float PITCH, float ROLL)
{
	this->Yaw = YAW;
	this->Pitch = PITCH;
	this->Roll = ROLL;
	UpdateCameraVectors();
}

void Lightning::Modules::Camera::setInputYaw(double input)
{
	input *= Sensitivity;
	Yaw += input;
	UpdateCameraVectors();
}

void Lightning::Modules::Camera::setInputPitch(double input, bool ConstraintPitch)
{
	SClamp AngleConstraint(-89.f, 89.f);
	input *= Sensitivity;
	Pitch += input;
	if (ConstraintPitch)
		Pitch = AngleConstraint.clamp(Pitch);
	UpdateCameraVectors();
}

void Lightning::Modules::Camera::setInputZoom(double offset_y, float deltaTime)
{
	SClamp depthConstraint(1.0f, 45.0f);
	FOV -= offset_y * deltaTime;
	FOV = depthConstraint.clamp(FOV);
}

float Lightning::Modules::Camera::getFOV()
{
    return FOV;
}

void Lightning::Modules::Camera::setFOV(float newFOV)
{
	this->FOV = newFOV;
}

glm::vec3 Lightning::Modules::Camera::getLocation()
{
    return Location;
}

void Lightning::Modules::Camera::setLocation(float x, float y, float z)
{
	this->Location.x = x;
	this->Location.y = y;
	this->Location.z = z;
}

void Lightning::Modules::Camera::UpdateCameraVectors()
{
	Front = MakeRotate(glm::vec3(Yaw, Pitch, 0.0f));
	// Also re-calculate the Right and Up vector
	Right = glm::normalize(glm::cross(Front, WorldUp));
	Up = glm::normalize(glm::cross(Right, Front));
}

Lightning::Modules::CameraHandler::CameraHandler()
{
	this->defaultCamera->setLocation(0, 1.f, 3.f);
}

Lightning::Modules::CameraHandler::CameraHandler(Camera* NewCamera)
{
	this->defaultCamera = NewCamera;
}

void Lightning::Modules::CameraHandler::setActiveCamera(Camera* NewCamera)
{
	this->defaultCamera = NewCamera;
}

Lightning::Modules::Camera* Lightning::Modules::CameraHandler::getActiveCamera()
{
	return defaultCamera;
}
