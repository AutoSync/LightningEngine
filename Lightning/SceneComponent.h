#pragma once
#include "Types.h"
namespace Lightning
{
	class SceneComponent
	{
		SceneComponent();
	public:
		Transform transform = Transform();
		V3 direction = V3();
		bool Visible = true;
		bool Activate = true;
		SceneComponent* Parent = NULL;
		vector<SceneComponent*> Childs = { NULL };
	private:
		Transform globalTransform = Transform();
		void InitializeSceneComponent();
		void UpdateByNodes();
	public:
		void SetTransform(Transform T);
		Transform GetTransform();
		void SetPosition(V3 P);
		V3 GetPosition();
		void SetRotation(V3 R);
		V3 GetRotation();
		void SetScale(V3 S);
		V3 GetScale();
	};
}

