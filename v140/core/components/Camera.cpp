#include "../../Camera.h"

void LightningEngine::Camera::_idc(const char* message)
{
	cout << "Camera: " << typeid(*this).name() << " " << message << endl;
}

glm::mat4 LightningEngine::Camera::GetViewMatrix()
{
    return viewMatrix;
}
glm::mat4 LightningEngine::Camera::getLookAt()
{
	viewMatrix = glm::lookAt(Position.GetGLM(), Position.GetGLM() + Front.GetGLM(), Up.GetGLM());
	return viewMatrix;
}
void LightningEngine::Camera::SetViewMatrix(glm::mat4 mat)
{
	viewMatrix = mat;
}
void LightningEngine::Camera::SetProjectionMatrix(glm::mat4 mat)
{
	projectionMatrix = mat;
}
glm::mat4 LightningEngine::Camera::getPespective()
{
	glm::mat4 _projection;
	if(usePerspective)
		_projection = glm::perspective(glm::radians(FOV), (float)Settings.width / (float)Settings.height, NearClip, FarClip);
	else
		_projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, NearClip, FarClip);
	return projection;
}
void LightningEngine::Camera::SetInputMovement(Direction direction, float deltaTime)
{
	float Velocity = MovementSpeed * deltaTime;
	string l = "", dir = "", message = "";
	
	switch (direction)
	{
	case LightningEngine::FORWARD:
		Position += Front * Velocity;
		l = to_string(Position.x);
		dir = "Forward";
		message = dir + ": " + l;
		_idc(message.c_str());
		break;
	case LightningEngine::BACKWARD:
		Position -= Front * Velocity;
		l = to_string(Position.x);
		dir = "Backward";
		message = dir + ": " + l;
		_idc(message.c_str());
		break;
	case LightningEngine::RIGHT:
		Position += Right * Velocity;
		l = to_string(Position.z);
		dir = "Right";
		message = dir + ": " + l;
		_idc(message.c_str());
		break;
	case LightningEngine::LEFT:
		Position -= Right * Velocity;
		l = to_string(Position.z);
		dir = "Left";
		message = dir + ": " + l;
		_idc(message.c_str());
		break;
	case LightningEngine::UP:
		Position += Up * Velocity;
		l = to_string(Position.y);
		dir = "Up";
		message = dir + ": " + l;
		_idc(message.c_str());
		break;
	case LightningEngine::DOWN:
		Position -= Up * Velocity;
		l = to_string(Position.y);
		dir = "Down";
		message = dir + ": " + l;
		_idc(message.c_str());
		break;
	}
}
void LightningEngine::Camera::SetRotation(float YAW, float PITCH, float ROLL)
{
	this->Yaw = YAW;
	this->Pitch = PITCH;
	this->Roll = ROLL;
	UpdateCameraVectors();
}
void LightningEngine::Camera::SetInputYaw(double input)
{
	input *= Sensitivity;
	Yaw += input;
	UpdateCameraVectors();
}
void LightningEngine::Camera::SetInputPitch(double input, bool ConstraintPitch)
{
	SClamp AngleConstraint(-89.f, 89.f);
	input *= Sensitivity;
	Pitch += input;
	if (ConstraintPitch)
		Pitch = AngleConstraint.clamp(Pitch);
	UpdateCameraVectors();
}
void LightningEngine::Camera::SetInputZoom(double offset_y, float deltatime)
{
	SClamp depthConstraint(1.0f, 45.0f);
	FOV -= offset_y * deltatime;
	FOV = depthConstraint.clamp(FOV);
}
float LightningEngine::Camera::GetFOV()
{
	return FOV;
}
void LightningEngine::Camera::SetFOV(float newFOV)
{
	this->FOV = newFOV;
}
LightningEngine::V3 LightningEngine::Camera::GetPosition()
{
	return Position;
}
void LightningEngine::Camera::SetPosition(float x, float y, float z)
{
	this->Position = V3(x, y, z);
}
void LightningEngine::Camera::Render()
{
	projection = glm::perspective(glm::radians(FOV),(float)Settings.width / (float)Settings.height,NearClip, FarClip);
	view = GetViewMatrix();
}
void LightningEngine::Camera::UpdateCameraVectors()
{
	Front = LightningEngine::MakeRotation(glm::vec3(Yaw, Pitch, 0.0f));
	// Also re-calculate the Right and Up vector
	Right = glm::normalize(glm::cross(Front.GetGLM(), WorldUp.GetGLM()));
	Up = glm::normalize(glm::cross(Right.GetGLM(), Front.GetGLM()));
}