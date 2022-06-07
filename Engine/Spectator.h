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
		Spectator(Camera& Camera);
		void AddInputMovement();
		Camera* GetCamera();
	};
}
