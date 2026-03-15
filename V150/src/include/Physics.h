// Physics.h — Lightning Engine 2D physics primitives.
// MotionPhysics: simple kinematic integration (no external library).
// Rect2D: axis-aligned bounding box (AABB) for collision and UI layout.

#pragma once
#include "Math.h"

namespace LightningEngine
{
	// -----------------------------------------------------------------------
	// Rect2D — axis-aligned rectangle (position + size in pixels)
	// -----------------------------------------------------------------------
	struct Rect2D
	{
		float x = 0.f;
		float y = 0.f;
		float w = 0.f;
		float h = 0.f;

		Rect2D() = default;
		Rect2D(float x, float y, float w, float h) : x(x), y(y), w(w), h(h) {}

		// Construct from centre + half-extents.
		static Rect2D FromCenter(float cx, float cy, float halfW, float halfH)
		{
			return { cx - halfW, cy - halfH, halfW * 2.f, halfH * 2.f };
		}

		Lightning::V2 Center()      const { return { x + w * 0.5f, y + h * 0.5f }; }
		Lightning::V2 TopLeft()     const { return { x, y }; }
		Lightning::V2 BottomRight() const { return { x + w, y + h }; }
		float Left()   const { return x; }
		float Right()  const { return x + w; }
		float Top()    const { return y; }
		float Bottom() const { return y + h; }

		bool Contains(Lightning::V2 point) const
		{
			return point.x >= x && point.x <= x + w
				&& point.y >= y && point.y <= y + h;
		}

		bool Overlaps(const Rect2D& other) const
		{
			return x < other.x + other.w && x + w > other.x
				&& y < other.y + other.h && y + h > other.y;
		}

		// Overlap depth on each axis (positive = overlap, negative = gap).
		Lightning::V2 OverlapDepth(const Rect2D& other) const
		{
			float ox = LMath::Min(x + w, other.x + other.w) - LMath::Max(x, other.x);
			float oy = LMath::Min(y + h, other.y + other.h) - LMath::Max(y, other.y);
			return { ox, oy };
		}

		bool operator==(const Rect2D& r) const { return x==r.x && y==r.y && w==r.w && h==r.h; }
	};

	// -----------------------------------------------------------------------
	// RigidBody2D — kinematic integration for a 2D object
	// -----------------------------------------------------------------------
	struct RigidBody2D
	{
		Lightning::V2 position    = {};
		Lightning::V2 velocity    = {};
		Lightning::V2 acceleration = {};

		float mass       = 1.f;   // kg (> 0)
		float drag       = 0.f;   // linear velocity damping per second [0..1]
		float restitution = 0.5f; // bounce coefficient [0=no bounce, 1=perfect]

		bool  useGravity  = false;
		float gravityScale = 1.f;

		// Pixels per second² (typical for a 2D game at 1px ≈ 1cm: 980 = ~9.8m/s²)
		static constexpr float kGravity = 980.f;

		// Apply a continuous force (Newtons). Accumulated until Integrate() is called.
		void ApplyForce(Lightning::V2 force)
		{
			if (mass > 0.f)
				acceleration += force / mass;
		}

		// Apply an instant velocity change (impulse / mass = Δv).
		void ApplyImpulse(Lightning::V2 impulse)
		{
			if (mass > 0.f)
				velocity += impulse / mass;
		}

		// Integrate: advance velocity and position by dt seconds.
		// Call once per frame from Update(dt).
		void Integrate(float dt)
		{
			if (useGravity)
				acceleration.y += kGravity * gravityScale;

			velocity += acceleration * dt;

			// Linear drag: v *= (1 - drag * dt), clamped so it never reverses.
			if (drag > 0.f)
			{
				float dampen = 1.f - LMath::Clamp(drag * dt, 0.f, 1.f);
				velocity *= dampen;
			}

			position += velocity * dt;

			// Reset per-frame accumulated forces (gravity re-applied each frame).
			acceleration = {};
		}

		// Move the body to a position and stop all motion.
		void Teleport(Lightning::V2 pos)
		{
			position     = pos;
			velocity     = {};
			acceleration = {};
		}

		// Current speed (magnitude of velocity).
		float Speed() const { return LMath::Length(velocity); }

		// Kinetic energy (½mv²).
		float KineticEnergy() const { return 0.5f * mass * LMath::LengthSq(velocity); }

		// Axis-aligned bounds given a half-size (for AABB collision).
		Rect2D Bounds(float halfW, float halfH) const
		{
			return Rect2D::FromCenter(position.x, position.y, halfW, halfH);
		}
	};

} // namespace LightningEngine
