#pragma once
#include "Camera.h"
#include "System.h"


namespace Lightning
{
	class Spectator
	{
	private:
		Inputs* Input = new Inputs(glfwGetCurrentContext());
		Camera* camera = new Camera();
	public:
		Spectator(Camera* cam);
		void CameraMovement(float dt);	
	};
}
