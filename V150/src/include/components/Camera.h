// Camera.h — Scene component. Controls the Renderer camera offset.
// Attach to any Node; the camera will follow that node's world position.
// Set worldW/worldH to clamp scrolling to the map boundaries.
//
// Usage:
//   auto* cam = cameraNode->AddComponent<CameraComponent>();
//   cam->target    = playerNode;
//   cam->smoothing = 0.12f;       // 0 = instant snap, higher = slower follow
//   cam->SetWorldBounds(1600.f, 1200.f, 800.f, 600.f); // worldW, worldH, screenW, screenH

#pragma once
#include "../Component.h"
#include "../Renderer.h"
#include "../Math.h"

namespace LightningEngine
{
	class CameraComponent : public Component
	{
	public:
		Node*  target    = nullptr; // Node to follow (usually the player)

		// How much of the world is visible (match your window size).
		float  screenW   = 800.f;
		float  screenH   = 600.f;

		// Map size — camera scrolling is clamped to [0, worldW-screenW] x [0, worldH-screenH].
		// Set both to 0 to disable clamping.
		float  worldW    = 0.f;
		float  worldH    = 0.f;

		// Lerp factor per second. 0 = instant, ~0.1 = smooth follow, 1 = never moves.
		float  smoothing = 0.f;

		// Fixed offset added to the followed position (e.g. look-ahead).
		Lightning::V2 offset = { 0.f, 0.f };

		void SetWorldBounds(float ww, float wh, float sw, float sh)
		{
			worldW = ww; worldH = wh;
			screenW = sw; screenH = sh;
		}

		void Update(float dt) override
		{
			if (!owner || !owner->renderer) return;

			Lightning::V2 dest = computeTarget();

			if (smoothing > 0.f)
			{
				float t = 1.f - std::pow(smoothing, dt / 1000.f);
				float cx = owner->renderer->GetCameraX();
				float cy = owner->renderer->GetCameraY();
				dest.x = LMath::Lerp(cx, dest.x, t);
				dest.y = LMath::Lerp(cy, dest.y, t);
			}

			owner->renderer->SetCameraOffset(dest.x, dest.y);
		}

	private:
		Lightning::V2 computeTarget() const
		{
			float cx = 0.f, cy = 0.f;

			if (target)
			{
				Lightning::V2 tp = target->WorldPosition();
				cx = tp.x + offset.x - screenW * 0.5f;
				cy = tp.y + offset.y - screenH * 0.5f;
			}

			// Clamp to world bounds if set
			if (worldW > 0.f) cx = LMath::Clamp(cx, 0.f, worldW - screenW);
			if (worldH > 0.f) cy = LMath::Clamp(cy, 0.f, worldH - screenH);

			return { cx, cy };
		}
	};

} // namespace LightningEngine
