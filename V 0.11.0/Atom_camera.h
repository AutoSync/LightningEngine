#ifndef CAMERAOBJECT_H
#define CAMERAOBJECT_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};
//Default Camera Values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

class CameraObject
{
public:
	//Camera Atributes
	glm::vec3 Location; // Relative Location
	glm::vec3 Front; // Front Camera
	glm::vec3 Up; // Camera Up Vector
	glm::vec3 Right; // Camera Right vector
	glm::vec3 WorldUp; // Absolute World Location
	// Euler Angles
	float Yaw; // Rotate camera left and right
	float Pitch; // Tilt camera up and down
	//Camera Properties
	float MouseSensitivity;
	float MovementSpeed;
	float Zoom;

	CameraObject(glm::vec3 location = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
		float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED),
		MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Location = location;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		UpdateCameraVectors();
	}
	CameraObject(float LocX, float LocY, float LocZ, float UpX, float UpY, float UpZ, float yaw, float pitch) : Front(glm::vec3(0.0f,0.0f,-1.0f)),
	MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Location = glm::vec3(LocX, LocY, LocZ);
		WorldUp = glm::vec3(UpX, UpY, UpZ);
		Yaw = yaw;
		Pitch = pitch;
		UpdateCameraVectors();
	}
	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(Location, Location + Front, Up);
	}
	// Manages pressed keys to move camera
	void ProcessKeyboard(Camera_Movement direction, float deltaTime)
	{
		float Velocity = MovementSpeed * deltaTime;
		if (direction == FORWARD)
			Location += Front * Velocity;
		if (direction == BACKWARD)
			Location -= Front * Velocity;
		if (direction == LEFT)
			Location -= Right * Velocity;
		if (direction == RIGHT)
			Location += Right * Velocity;
		if (direction == UP)
			Location += Up * Velocity;
		if (direction == DOWN)
			Location -=  Up * Velocity;
	}
	void ProcessMouseMovement(float offset_X, float offset_Y, GLboolean ConstrainPitch = true)
	{
		offset_X *= MouseSensitivity;
		offset_Y *= MouseSensitivity;

		Yaw += offset_X;
		Pitch += offset_Y;

		if (ConstrainPitch)
		{
			if (Pitch > 89.0f)
				Pitch = 89.0f;
			if (Pitch < -89.0f)
				Pitch = -89.0f;
		}
		UpdateCameraVectors();
	}
	void ProcessMouseScroll(float offset_Y)
	{
		if (Zoom >= 1.0f && Zoom <= 45.0f)
			Zoom -= offset_Y;
		if (Zoom <= 1.0f)
			Zoom = 1.0f;
		if (Zoom >= 45.0f)
			Zoom = 45.0f;
	}
private:
	// Calculates the front vector from the Camera's (updated) Euler Angles
	void UpdateCameraVectors()
	{
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front = glm::normalize(front);
		// Also re-calculate the Right and Up vector
		Right = glm::normalize(glm::cross(Front, WorldUp));
		Up = glm::normalize(glm::cross(Right, Front));
	}
};
#endif
