// ParallaxLayer.h — Multi-layer parallax background renderer.
//
// Each layer has a texture and a scroll factor:
//   0.0 = fixed (doesn't move with camera)
//   0.5 = moves at half camera speed (far background)
//   1.0 = moves at camera speed (same layer as world)
//  >1.0 = moves faster than camera (foreground elements)
//
// Layers are drawn back-to-front.  Call Draw() once per frame, passing the
// current camera X offset and window size.
//
// Usage:
//   ParallaxBackground bg;
//   bg.AddLayer(&skyTex,     0.0f);  // fixed sky
//   bg.AddLayer(&cloudsTex,  0.2f);  // slow clouds
//   bg.AddLayer(&hillsTex,   0.5f);  // medium hills
//   bg.AddLayer(&groundTex,  0.8f);  // near ground
//
//   // In Render():
//   bg.Draw(renderer, renderer.GetCameraX(), screenW, screenH);
//
// ParallaxBackgroundComponent: attach to a Node for automatic draw.
#pragma once
#include <vector>
#include <SDL3/SDL.h>
#include "Renderer.h"
#include "Texture.h"
#include "Component.h"

namespace LightningEngine {

struct ParallaxLayer {
    Texture* texture      = nullptr;   // not owned
    float    scrollFactor = 1.f;       // 0=fixed, 1=moves with camera
    float    offsetY      = 0.f;       // vertical shift in pixels
    float    scaleX       = 1.f;       // horizontal stretch multiplier (>1 = zoom out)
    float    scaleY       = 1.f;       // vertical stretch multiplier
};

class ParallaxBackground {
public:
    std::vector<ParallaxLayer> layers;

    void AddLayer(Texture* tex, float scrollFactor,
                  float offsetY = 0.f, float scaleX = 1.f, float scaleY = 1.f)
    {
        layers.push_back({ tex, scrollFactor, offsetY, scaleX, scaleY });
    }

    void Clear() { layers.clear(); }

    // Draw all layers.
    // cameraX    — current horizontal camera world offset.
    // screenW/H  — viewport pixel dimensions.
    void Draw(Renderer& r, float cameraX, float screenW, float screenH) const
    {
        for (const auto& layer : layers) {
            if (!layer.texture || !layer.texture->IsValid()) continue;

            float texW = (float)layer.texture->GetWidth()  * layer.scaleX;
            float texH = (float)layer.texture->GetHeight() * layer.scaleY;

            // Horizontal parallax offset (camera pans right → layer shifts left).
            float rawX = -cameraX * layer.scrollFactor;

            // Wrap into [0, texW) so the texture tiles seamlessly.
            float startX = fmodf(rawX, texW);
            if (startX > 0.f) startX -= texW;

            // Vertical: anchor bottom of layer to bottom of screen.
            float startY = screenH - texH + layer.offsetY;

            // Draw enough tiles to fill the screen width.
            r.BeginScreenSpace();
            float x = startX;
            while (x < screenW) {
                r.DrawTexture(*layer.texture, x, startY, texW, texH);
                x += texW;
            }
            r.EndScreenSpace();
        }
    }
};

// ── ParallaxBackgroundComponent ───────────────────────────────────────────────

class ParallaxBackgroundComponent : public Component {
public:
    ParallaxBackground bg;
    float screenW = 1280.f;
    float screenH = 720.f;

    void Render() override
    {
        if (!owner || !owner->renderer) return;
        // Update screen size from window each frame.
        int sw = (int)screenW, sh = (int)screenH;
        if (owner->renderer->GetWindow())
            SDL_GetWindowSize(owner->renderer->GetWindow(), &sw, &sh);

        bg.Draw(*owner->renderer,
                owner->renderer->GetCameraX(),
                (float)sw, (float)sh);
    }
};

} // namespace LightningEngine
