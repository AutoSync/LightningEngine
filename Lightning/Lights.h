#pragma once
#include "Types.h"
#include "SceneComponent.h"
#include "Shader.h"

namespace Lightning
{
	class Light : SceneComponent
	{
	public:
		LightType Type = LightType::Direct;
		Mobility Mobility = Mobility::Mobile;
		Transform transform;
		LinearColor diffuse = 1.0f;				//Light color		
		LinearColor ambient = 0.25f;			//Ambiant color	
		LinearColor specular = 0.5f;			//Specular color		
		float constant = 1.0f;					//Constant value
		float linear = 0.5f;
		float quadratic = 0.032f;
		float internalCone = 12.f;
		float cutOff = glm::cos(glm::radians(internalCone));
		float externalCone = 15.0f;
		float outerCutOff = glm::cos(glm::radians(externalCone));
		bool isVisibility = true;
	public:
		Light();
		Light(Transform T, LightType type);
		void Render(Shader* shader);
		void Render(int iterator, Shader* shader);
		void CreateDirectionLight();
		void CreateSpotlight();
		void CreatePoint();
	private:
		void InitializeLight(Transform T, LightType type);
		
	};

	
}

