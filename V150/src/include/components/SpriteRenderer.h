// SpriteRenderer.h — Scene component. Renders a Texture at the Node's world position.
// Category: Scene (injects data into the render pipeline).
//
// Usage:
//   auto* sr = node->AddComponent<SpriteRendererComponent>();
//   sr->texture = &myTexture;        // pointer — node does NOT own the texture
//   sr->SetSize(64.f, 64.f);         // override natural size (0 = natural)
//   sr->pivot = { 0.5f, 0.5f };      // 0,0 = top-left  1,1 = bottom-right
//   sr->color = { 255, 255, 255, 200 }; // tint + alpha

#pragma once
#include "../Component.h"
#include "../Texture.h"
#include "../Renderer.h"

namespace LightningEngine
{
	class SpriteRendererComponent : public Component
	{
	public:
		Texture*       texture = nullptr; // not owned — managed by caller
		float          width   = 0.f;     // 0 = use natural texture width
		float          height  = 0.f;     // 0 = use natural texture height
		Lightning::V2  pivot   = { 0.5f, 0.5f }; // normalised pivot (centre)

		// Tint (white = no tint). Applied via Renderer::SetDrawColor.
		Uint8 colorR = 255, colorG = 255, colorB = 255, colorA = 255;

		// Optional UV region for sprite-sheet sub-sprites (0..1).
		float u0 = 0.f, v0 = 0.f, u1 = 1.f, v1 = 1.f;

		void SetSize(float w, float h)  { width = w; height = h; }
		void SetColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255)
		{
			colorR = r; colorG = g; colorB = b; colorA = a;
		}
		// Set UV region for a cell inside a sprite sheet.
		// sheetCols / sheetRows = total columns/rows; col/row = 0-based index.
		void SetSheetFrame(int col, int row, int sheetCols, int sheetRows)
		{
			float fw = 1.f / (float)sheetCols;
			float fh = 1.f / (float)sheetRows;
			u0 = col       * fw;
			v0 = row       * fh;
			u1 = (col + 1) * fw;
			v1 = (row + 1) * fh;
		}

		void Render() override
		{
			if (!texture || !owner || !owner->renderer) return;

			float w = width  > 0.f ? width  : (float)texture->GetWidth();
			float h = height > 0.f ? height : (float)texture->GetHeight();

			Lightning::V2 wp = owner->WorldPosition();
			float x = wp.x - w * pivot.x;
			float y = wp.y - h * pivot.y;

			// Apply scale from node transform
			float sx = owner->transform.Scale.x;
			float sy = owner->transform.Scale.y;
			w *= sx; h *= sy;

			owner->renderer->SetDrawColor(colorR, colorG, colorB, colorA);

			if (u0 == 0.f && v0 == 0.f && u1 == 1.f && v1 == 1.f)
				owner->renderer->DrawTexture(*texture, x, y, w, h);
			else
				owner->renderer->DrawTextureRegion(*texture, x, y, w, h, u0, v0, u1, v1);
		}
	};

} // namespace LightningEngine
