// RigidBody2D.h — Logic component. Wraps a RigidBody2D and keeps the Node's
// Transform in sync with the physics simulation.
// Category: Logic (drives node position each frame via physics integration).
//
// Usage:
//   auto* rb = node->AddComponent<RigidBody2DComponent>();
//   rb->body.useGravity   = true;
//   rb->body.mass         = 1.f;
//   rb->body.drag         = 0.1f;
//   rb->body.restitution  = 0.4f;
//
//   // From a script / another component:
//   rb->body.ApplyImpulse({ 0.f, -600.f }); // jump
//   rb->body.ApplyForce({ 200.f, 0.f });     // push

#pragma once
#include "../Component.h"
#include "../Physics.h"

namespace LightningEngine
{
	class RigidBody2DComponent : public Component
	{
	public:
		RigidBody2D body;

		// Whether to write simulation results back to Node::transform each frame.
		// Set to false if you want to drive the body position manually.
		bool syncTransform = true;

		void OnAttach() override
		{
			// Initialise body position from the node's current transform.
			if (owner)
			{
				body.position = {
					owner->transform.Position.x,
					owner->transform.Position.y
				};
			}
		}

		void Update(float dt) override
		{
			body.Integrate(dt);

			if (syncTransform && owner)
			{
				owner->transform.Position.x = body.position.x;
				owner->transform.Position.y = body.position.y;
			}
		}

		// Helper — axis-aligned bounds using node transform scale as half-extents.
		// Assumes Scale.x / Scale.y = sprite half-width / half-height in pixels.
		Rect2D Bounds() const
		{
			if (!owner) return {};
			float hw = owner->transform.Scale.x * 0.5f;
			float hh = owner->transform.Scale.y * 0.5f;
			return body.Bounds(hw, hh);
		}
	};

} // namespace LightningEngine
