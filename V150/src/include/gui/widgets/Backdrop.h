// Backdrop.h — Semi-transparent overlay, shadow, dim, or blur layer.
//
// Mode::DIM     — full-rect solid color at reduced alpha (modal overlay)
// Mode::SHADOW  — drop shadow: offset dark rect rendered behind children
// Mode::OUTLINE — subtle glow/outline border around content area
// Mode::BLUR    — renders a blurred copy of a source Framebuffer behind children
//                 Requires blurSource to be set each frame before Render.
//
// BLUR usage:
//   // Setup:
//   Framebuffer sceneFB, blurFB;
//   sceneFB.Create(renderer, W, H);
//   blurFB.Create(renderer, W, H);
//   bd->mode       = Backdrop::Mode::BLUR;
//   bd->blurSource = &sceneFB;
//   bd->blurDest   = &blurFB;
//   bd->blurRadius = 6.f;
//
//   // Each frame:
//   renderer.BeginRenderToTexture(sceneFB);
//   renderScene();
//   renderer.EndRenderToTexture();
//   renderer.BlurTexture(blurFB, sceneFB.GetTexture(), bd->blurRadius);
//   // then render UI normally
#pragma once
#include "../Widget.h"
#include "../TitanStyle.h"
#include "../TitanFont.h"
#include "../../Framebuffer.h"

namespace Titan {

class Backdrop : public Widget {
public:
    enum class Mode { DIM, SHADOW, OUTLINE, BLUR };

    Mode  mode      = Mode::DIM;
    Uint8 dimAlpha  = 160;           // DIM: overlay alpha
    Uint8 dimR=0, dimG=0, dimB=0;    // DIM: overlay color (default black)

    // SHADOW settings
    float shadowOffX = 4.f;
    float shadowOffY = 4.f;
    float shadowBlur = 0.f;          // reserved, not rendered yet
    Uint8 shadowAlpha = 120;

    // OUTLINE settings
    float outlineThick = 2.f;
    Uint8 outlineR=85, outlineG=148, outlineB=240, outlineA=200; // accent blue

    // BLUR settings — set each frame before Render
    LightningEngine::Framebuffer* blurSource = nullptr;  // pre-blurred FB (output of BlurTexture)
    float        blurTintA  = 255.f;    // alpha to blend the blurred layer (0-255)

    // Blocks all input when DIM mode is active (modal behaviour).
    bool  blockInput = true;

    Backdrop() = default;
    // DIM fullscreen constructor
    Backdrop(float x, float y, float w, float h, Uint8 alpha = 160)
        : dimAlpha(alpha)
    { this->x=x; this->y=y; this->w=w; this->h=h; mode=Mode::DIM; }

    bool ProcessInput(float mx, float my, bool ldown, bool lclick, bool lrelease,
                      TitanUI* ui, float ox, float oy) override
    {
        if (!visible || !enabled) return false;
        float ax = ox+x, ay = oy+y;

        // Forward to children first (the dialog/window on top)
        for (int i = (int)children.size()-1; i >= 0; i--) {
            if (children[i]->ProcessInput(mx, my, ldown, lclick, lrelease,
                                          ui, ax, ay))
                return true;
        }

        // Block underlying input when in DIM mode
        if (mode == Mode::DIM && blockInput) {
            bool inRect = mx >= ax && mx < ax+w && my >= ay && my < ay+h;
            if (inRect) return ldown || lclick || lrelease;
        }
        return false;
    }

    void Render(LightningEngine::Renderer& r, TitanFont& f, float ox, float oy) override
    {
        if (!visible) return;
        float ax = ox+x, ay = oy+y;

        switch (mode) {
        case Mode::DIM:
            r.SetDrawColor(dimR, dimG, dimB, dimAlpha);
            r.FillRect(ax, ay, w, h);
            break;

        case Mode::SHADOW:
            // Render a blurred-ish shadow by stacking translucent rects
            for (float i = 0.f; i < 4.f; i += 1.f) {
                Uint8 a = (Uint8)(shadowAlpha * (1.f - i / 4.f));
                r.SetDrawColor(0, 0, 0, a);
                r.FillRect(ax + shadowOffX + i,
                           ay + shadowOffY + i,
                           w - i * 2.f,
                           h - i * 2.f);
            }
            break;

        case Mode::OUTLINE:
            for (float t = 0.f; t < outlineThick; t += 1.f) {
                r.SetDrawColor(outlineR, outlineG, outlineB,
                               (Uint8)(outlineA * (1.f - t / outlineThick)));
                r.DrawRect(ax - t, ay - t, w + t * 2.f, h + t * 2.f);
            }
            break;

        case Mode::BLUR:
            // Draw the pre-blurred framebuffer clipped to this rect.
            // The caller must have called renderer.BlurTexture(dest, scene) already.
            if (blurSource && blurSource->IsValid()) {
                float bw = (float)blurSource->GetWidth();
                float bh = (float)blurSource->GetHeight();
                // Compute UV region corresponding to this widget's screen rect
                float u0_ = ax / bw, v0_ = ay / bh;
                float u1_ = (ax+w) / bw, v1_ = (ay+h) / bh;
                r.SetDrawColor(255, 255, 255, (Uint8)blurTintA);
                r.DrawTextureRegion(blurSource->GetTexture(),
                                    ax, ay, w, h,
                                    u0_, v0_, u1_, v1_);
            } else {
                // Fallback: dim if blur source not ready
                r.SetDrawColor(0, 0, 0, dimAlpha);
                r.FillRect(ax, ay, w, h);
            }
            break;
        }

        // Children rendered on top of the backdrop
        for (auto& c : children) c->Render(r, f, ax, ay);
    }
};

} // namespace Titan
