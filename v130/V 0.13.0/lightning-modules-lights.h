#pragma once
#include "random-rankit.h"
#include "lightning-frontend.h"
#include "equinox-bake-opengl.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace Equinox;
using namespace Random::Structs;
typedef glm::mat4 mat4;
typedef glm::vec3 vec3;
namespace Lightning
{
	namespace Modules
	{
		class Lights
		{
		public:
			Lights();
			Lights(const Lightning::Front::ELightMode LightMode);
			~Lights();
			//creates and defines a direct light
			void createLightDirection();
			//creates and defines a point of light
			void createLightPoint();
			//creates and defines a spotlight
			void createSpotLight();
			//Renders the light in the shader
			void useLight(Shader* shader);
			void useLight(int iterator, Shader* shader);
			// Defines whether the light is visible or not
			void setVisibility(bool newVisibility);
			void setLightMode(Lightning::Front::ELightMode lightmode);
			void setTransform(Transform newT);
			Transform getTransform();
			void setRelativeLocation(v3 NewLocation);
			v3 getRelativeLocation();
			void setRelativeRotation(r3 NewRotation);
			r3 getRelativeRotation();
			void setRelativeScale(v3 NewScale);
			v3 getRelativeScale();
			void setLightColor(c3 newColor);
			c3 getLightColor();
			void setAmbientColor(c3 newAmbient);
			c3 getAmbientColor();
			void setSpecularColor(c3 newSpecular);
			c3 getSpecularColor();
			void setQuadratic(float newQuadratic);
			float getQuadratic();
			void setLinear(float newLinear);
			float getLinear();
		private:
			Lightning::Front::ELightMode LightMode = Lightning::Front::ELightMode::LIGHT_DIRECT;
			Lightning::Front::ELightState LightState = Lightning::Front::ELightState::STATIC;
			Transform transform;
			//Light color
			c3 diffuse = 1.0f;
			//Ant color
			c3 ambient = 0.25f;
			//Specular color
			c3 specular = 0.5f;
			//Constant value
			float constant = 1.0f;
			float linear = 0.5f;
			float quadratic = 0.032f;
			float internalCone = 12.f;
			float cutOff = glm::cos(glm::radians(internalCone));
			float externalCone = 15.0f;
			float outerCutOff = glm::cos(glm::radians(externalCone));

			bool isVisibility = true;
		};
		
	}
}
