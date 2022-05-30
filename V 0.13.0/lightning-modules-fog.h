#pragma once
#include "random-rankit.h"
#include "equinox-bake-opengl.h"

using namespace Random::Structs;
using namespace Equinox;

namespace Lightning
{
	namespace Modules
	{
		class Fog
		{
		public:
			Fog();
			Fog(float newNear, float newFar);
			~Fog();
			void use(Shader* shader);
			void setColorFog(c3 newColor);
			void setColorFog(int R, int G, int B);
			c3 getColorFog();
			void setDensityFog(float newDensity);
			void setStartFog(float distance);
			void setEndFog(float distance);
		private:
			bool visibility = true;
			float fogNear = 0.1f;
			float fogFar = 100.0f;
			float density = 1.0f;
			c3 colorFog = c3(255);
		};
	}
}
