#pragma once
#include "Types.h"
#include "Camera.h"
#include "Inputs.h"
#include "System.h"
#include "Match.h"

namespace Lightning
{
	class Spectator
	{
	private:
		Camera* camera;
		Inputs* Input;
	public:
		//Constructor
		Spectator(Camera& Camera);
		//Camera Movement
		void AddInputMovement();
		//Get Camera
		Camera* GetCamera();
		//Update Camera view
		void Update();
		//Change Perspective
		void SetPespective(bool Perspective);
	private:
		
	};
}
