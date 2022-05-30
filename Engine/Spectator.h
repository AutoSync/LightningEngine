#pragma once
#include "Types.h"
#include "System.h"
#include "Camera.h"
#include "Inputs.h"

namespace Lightning
{
	class Spectator
	{
	private:
		Camera* camera;
		Inputs* Input;
	public:
		Spectator(Camera* Camera);
		void AddInputMovement(float dt);
	};
}
