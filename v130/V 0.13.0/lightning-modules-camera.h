#pragma once
#include "random-rankit.h"
#include "lightning-backend.h"
// MATH
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

typedef glm::vec3 vec3;
typedef glm::mat4 mat4;

using namespace Random::Enums;
using namespace Random::Structs;
namespace Lightning
{
	namespace Modules
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

			vec3 Location;								// Relative Location
			vec3 Front;									// Front Camera
			vec3 Up;									// Camera Up Vector
			vec3 Right;									// Camera Right vector
			vec3 WorldUp;								// Absolute World Location
			// Euler Angles

			float Yaw		= -90.0f;	// Rotate camera left and right
			float Pitch		=	0.0f;	// Tilt camera up and down
			float Roll		=   0.0f;	//
			//Camera Properties

			float Sensitivity		= 0.1f;						//Input Movement
			float MovementSpeed		= 1.0f;						//MovementSpeed
			float FOV				= 45.f;						//Field of View
		public:
			Camera(vec3 location = vec3(0.0f), vec3 up = vec3(0.0f, 1.0f, 0.0f), float yaw = DefaultYaw,
				float pitch = DefaultPitch) : Front(0.0f, 0.0f, -1.0f), MovementSpeed(DefaultSpeed),
				Sensitivity(DefaultSensitivity), FOV(DefaultFOV)
			{
				this->Location = location;
				this->WorldUp = up;
				this->Yaw = yaw;
				this->Pitch = pitch;
				UpdateCameraVectors();
			}
			Camera(float LocationX, float LocationY, float LocationZ, float UpX, float UpY, float UpZ, float yaw, float pitch)
				: Front(vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(DefaultSpeed), Sensitivity(DefaultSensitivity)
			{
				this->Location = glm::vec3(LocationX, LocationY, LocationZ);
				this->WorldUp = glm::vec3(UpX, UpY, UpZ);
				this->Yaw = yaw;
				this->Pitch = pitch;
				UpdateCameraVectors();
			}
			~Camera(){ /*DESTRUCTOR*/ }
			mat4 GetViewMatrix();
			void SetInputMovement(EVectorMovement direction, float deltaTime);
			void SetRotation(float YAW, float PITCH, float ROLL);
			void setInputYaw(double input);
			void setInputPitch(double input, bool ConstraintPitch = true);
			void setInputZoom(double offset_y, float deltatime);
			float getFOV();
			void setFOV(float newFOV);
			vec3 getLocation();
			void setLocation(float x, float y, float z);
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
}
