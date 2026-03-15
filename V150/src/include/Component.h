// Component.h — Base class for all engine components.
// Attach to a GameObject via GameObject::AddComponent<T>().
//
// CLASS HIERARCHY:
// Window -> GameInstance -> Level -> GameObject -> [ COMPONENT ]

#pragma once

namespace LightningEngine
{
	class Node; // forward

	class Component
	{
	public:
		Node* owner   = nullptr;
		bool        enabled = true;

		virtual ~Component() = default;

		// Called once after the component is attached to a GameObject.
		virtual void OnAttach()  {}

		// Called once when the component is removed or the GameObject is destroyed.
		virtual void OnDetach()  {}

		// Called every frame. dt = delta time in milliseconds.
		virtual void Update(float dt) {}

		// Called every frame after Update — use renderer to draw.
		virtual void Render()    {}
	};

} // namespace LightningEngine
