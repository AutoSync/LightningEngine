#pragma once
#include "Shader.h"
#include "Types.h"
namespace Lightning
{
	class Cubes
	{
	public:
		Cubes();
		void Init();
		void Render(Shader* shader);
	private:
		float* vertices[];
	};

}
