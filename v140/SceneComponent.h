#pragma once
#include "Types.h"
#include "GameNode.h"
#include "Inputs.h"
#include <memory>


namespace LightningEngine
{
	// Legacy transform component. GameNode owns the scene hierarchy; this class
	// remains the compatibility base for MeshComponent and Light.
	class SceneComponent : public GameComponent
	{
	public:
		SceneComponent();
		Transform transform = Transform();
		V3 direction = V3();
		bool Visible = true;
		bool Active = false;
		// Kept for source compatibility with the original v0.14 API. New code
		// must use GameNode::AddChild and GameNode::GetParent.
		SceneComponent* Parent = nullptr;
		vector<SceneComponent*> Childs;
	protected:
		Transform globalTransform = Transform();
		void InitializeSceneComponent();
		void UpdateByNodes();
		std::unique_ptr<Inputs> Input;
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

