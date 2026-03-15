// BoxRenderer.h — Scene component. Renders a solid colored rectangle at the Node's
// world position. Useful for prototyping before sprites are ready.
// Category: Scene.
//
// Usage:
//   auto* box = node->AddComponent<BoxRendererComponent>();
//   box->SetSize(48.f, 48.f);
//   box->SetColor(220, 50, 50);       // red
//   box->pivot = { 0.5f, 0.5f };      // center

#pragma once
#include "../Component.h"
#include "../Renderer.h"

namespace LightningEngine
{
	class BoxRendererComponent : public Component
	{
	public:
		float         width  = 32.f;
		float         height = 32.f;
		Lightning::V2 pivot  = { 0.5f, 0.5f }; // 0,0=top-left  1,1=bottom-right

		Uint8 colorR = 255, colorG = 255, colorB = 255, colorA = 255;

		// Optional outline (drawn as 1px border when true).
		bool  outline      = false;
		Uint8 outlineR = 255, outlineG = 255, outlineB = 255, outlineA = 255;

		void SetSize(float w, float h)  { width = w; height = h; }
		void SetColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255)
		{
			colorR = r; colorG = g; colorB = b; colorA = a;
		}
		void SetOutline(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255)
		{
			outline = true;
			outlineR = r; outlineG = g; outlineB = b; outlineA = a;
		}

		void Render() override
		{
			if (!owner || !owner->renderer) return;

			float sx = owner->transform.Scale.x;
			float sy = owner->transform.Scale.y;
			float w  = width  * (sx > 0.f ? sx : 1.f);
			float h  = height * (sy > 0.f ? sy : 1.f);

			Lightning::V2 wp = owner->WorldPosition();
			float x = wp.x - w * pivot.x;
			float y = wp.y - h * pivot.y;

			owner->renderer->SetDrawColor(colorR, colorG, colorB, colorA);
			owner->renderer->FillRect(x, y, w, h);

			if (outline)
			{
				owner->renderer->SetDrawColor(outlineR, outlineG, outlineB, outlineA);
				owner->renderer->DrawRect(x, y, w, h);
			}
		}

		// Returns the screen-space AABB of this box.
		LightningEngine::Rect2D GetBounds() const
		{
			if (!owner) return {};
			Lightning::V2 wp = owner->WorldPosition();
			float sx = owner->transform.Scale.x;
			float sy = owner->transform.Scale.y;
			float w  = width  * (sx > 0.f ? sx : 1.f);
			float h  = height * (sy > 0.f ? sy : 1.f);
			return { wp.x - w * pivot.x, wp.y - h * pivot.y, w, h };
		}
	};

} // namespace LightningEngine
