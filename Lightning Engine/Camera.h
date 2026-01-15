#pragma once
#include "Types.h"
#include "SceneComponent.h"
#include "Msg.h"

namespace Lightning
{
	const float DefaultYaw = -90.0f;			//Default Values
	const float DefaultPitch = 0.0f;			//Default Values
	const float DefaultSpeed = 2.5f;			//Default Values
	const float DefaultSensitivity = 0.1f;		//Default Values
	const float DefaultFOV = 45.0f;				//Default Values
	
	class Camera
	{
	public:
		//Object Params
		//Camera Atributes

		V3 Position;								// Relative Location
		V3 Front;									// Front Camera
		V3 Up;										// Camera Up Vector
		V3 Right;									// Camera Right vector
		V3 WorldUp;									// Absolute World Location
		// Euler Angles	

		float Yaw = -90.0f;	// Rotate camera left and right
		float Pitch = 0.0f;	// Tilt camera up and down
		float Roll = 0.0f;	//
		//Camera Properties

		float Sensitivity = 0.1f;						//Input Movement
		float MovementSpeed = 1.0f;						//MovementSpeed
		float FOV = 45.f;								//Field of View
	public:
		Camera(V3 position = V3(0.0f), V3 up = V3(0.0f, 1.0f, 0.0f), float yaw = DefaultYaw,
			float pitch = DefaultPitch) : Front(0.0f, 0.0f, -1.0f), MovementSpeed(DefaultSpeed),
			Sensitivity(DefaultSensitivity), FOV(DefaultFOV)
		{
			this->Position = position;
			this->WorldUp = up;
			this->Yaw = yaw;
			this->Pitch = pitch;
			UpdateCameraVectors();
		}
		Camera(float PositionX, float PositionY, float PositionZ, float UpX, float UpY, float UpZ, float yaw, float pitch)
			: Front(V3(0.0f, 0.0f, -1.0f)), MovementSpeed(DefaultSpeed), Sensitivity(DefaultSensitivity)
		{
			this->Position = glm::vec3(PositionX, PositionY, PositionZ);
			this->WorldUp = glm::vec3(UpX, UpY, UpZ);
			this->Yaw = yaw;
			this->Pitch = pitch;
			UpdateCameraVectors();
		}
		~Camera() { /*DESTRUCTOR*/ }
		glm::mat4 GetViewMatrix();
		void SetInputMovement(Direction direction, float deltaTime);
		void SetRotation(float YAW, float PITCH, float ROLL);
		void SetInputYaw(double input);
		void SetInputPitch(double input, bool ConstraintPitch = true);
		void SetInputZoom(double offset_y, float deltatime);
		float GetFOV();
		void SetFOV(float newFOV);
		V3 GetLocation();
		void SetLocation(float x, float y, float z);
	private:
		void UpdateCameraVectors();

	};

	class CameraHandler
	{
	public:
		CameraHandler();
		CameraHandler(Camera* NewCamera);
		~CameraHandler() { delete this; };
		void setActiveCamera(Camera* NewCamera);
		Camera* getActiveCamera();
	private:
		Camera* defaultCamera;
	};
}