#pragma once
#include "Types.h"
#include "Inputs.h"
namespace Lightning
{
	class SceneComponent
	{
	public:
		SceneComponent();
		Transform transform = Transform();
		V3 direction = V3();
		bool Visible = true;
		bool Active = false;
		SceneComponent* Parent = NULL;
		vector<SceneComponent*> Childs = { NULL };
	protected:
		Transform globalTransform = Transform();
		void InitializeSceneComponent();
		void UpdateByNodes();
		Inputs* input = new Inputs(glfwGetCurrentContext());
	public:
		void SetRelativeTransform(Transform T);
		Transform GetRelativeTransform();
		void SetRelativePosition(V3 P);
		V3 GetRelativePosition();
		void SetRelativeRotation(V3 R);
		V3 GetRelativeRotation();
		void SetRelativeScale(V3 S);
		V3 GetRelativeScale();

	};
}

