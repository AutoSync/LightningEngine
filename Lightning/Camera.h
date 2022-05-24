#pragma once
#include "Types.h"
#include "SceneComponent.h"

namespace Lightning
{
	class Camera : SceneComponent
	{
	private:
		glm::vec3 direction = glm::normalize(transform.Position.glm - target.glm);
		glm::vec3 right = glm::normalize(glm::cross(Up.glm, direction));
		glm::vec3 CameraUp = glm::cross(direction, right);
		V3 front;
		V3 worldUp;
	public:
		V3 target = V3();
		V3 Up = V3(0.f, 1.f, 0.f);
		glm::mat4 view = glm::mat4(0.0f);

		//Euler Angles
		V3 Rotation = V3(-90.f, 0.f, 0.f);
		//Camera Properties

		float Sensitivity = 0.1f;						//Input Movement
		float MovementSpeed = 1.0f;						//MovementSpeed
		float FOV = 45.f;								//Field of View

	public:
		Camera();
		Camera(Transform T);
		void Render();
		void CameraMovement(V3 position);
		void CameraRotation(V3 rotation);
	private:
		void InitializeCamera(Transform T);
		void UpdateCameraVectors();

	};
}