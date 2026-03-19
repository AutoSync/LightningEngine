// Viewport2D.h — Embeds a 2D render region inside the Titan GUI.
//
// Two rendering modes:
//
//  1. INLINE (default, framebuffer=nullptr):
//     The onRender callback draws directly to the active render target
//     (swapchain or parent framebuffer). Fast, no extra GPU texture.
//     Camera offset is applied automatically if useAutoOffset=true.
//
//  2. ISOLATED (framebuffer != nullptr):
//     Each frame the viewport renders into its own Framebuffer, then
//     blits the result to the screen. True isolation: scene objects
//     never bleed outside the widget rect.
//     Call viewport->Resize(renderer) each frame if the widget resizes.
//
// Usage (inline):
//   auto* vp = panel->Add<Viewport2D>(x, y, w, h);
//   vp->onRender = [](Renderer& r, float x, float y, float w, float h) {
//       r.SetCameraOffset(...);
//       // draw world objects
//   };
//
// Usage (isolated with Framebuffer):
//   auto* vp = panel->Add<Viewport2D>(x, y, 640, 360);
//   vp->fb.Create(renderer, 640, 360);
//   vp->onRender = ...;
#pragma once
#include <functional>
#include "../Widget.h"
#include "../TitanStyle.h"
#include "../TitanFont.h"
#include "../../Renderer.h"
#include "../../Framebuffer.h"

namespace Titan {

class Viewport2D : public Widget {
public:
    // Called every frame inside Render, with the widget's screen rect.
    // Renderer is in screen-space mode when the callback fires — call
    // r.SetCameraOffset() then draw world objects normally.
    std::function<void(LightningEngine::Renderer& r,
                       float x, float y, float w, float h)> onRender;

    bool  showBorder  = true;
    bool  showBg      = true;
    bool  showNoRender = true;  // placeholder text when onRender is null

    // Optional camera target position (world units) — set each frame by owner.
    float camWorldX     = 0.f;
    float camWorldY     = 0.f;
    bool  useAutoOffset = false;

    // Optional isolated framebuffer (create and own externally or via fb.Create).
    // When valid, onRender draws into this FB; the FB is blitted to the widget rect.
    LightningEngine::Framebuffer fb;

    Viewport2D() = default;
    Viewport2D(float x, float y, float w, float h)
    { this->x=x; this->y=y; this->w=w; this->h=h; }

    // Resize the internal Framebuffer to the current widget size (call each frame if needed).
    void SyncFramebuffer(LightningEngine::Renderer& r)
    {
        if (w > 0 && h > 0)
            fb.Resize(r, (int)w, (int)h);  // NOLINT: fb is LightningEngine::Framebuffer
    }

    void Render(LightningEngine::Renderer& r, TitanFont& f, float ox, float oy) override
    {
        if (!visible) return;
        float ax = ox+x, ay = oy+y;
        const Style& s = gStyle;

        if (onRender) {
            if (fb.IsValid()) {
                // ── ISOLATED mode: render to FB, then blit ──
                r.EndScreenSpace(); // world-space for scene
                r.BeginRenderToTexture(fb);

                if (useAutoOffset) r.SetCameraOffset(camWorldX, camWorldY);
                onRender(r, 0.f, 0.f, (float)fb.GetWidth(), (float)fb.GetHeight());

                r.EndRenderToTexture();
                r.BeginScreenSpace();

                // Blit FB to widget rect
                r.SetDrawColor(255, 255, 255);
                r.DrawTexture(fb.GetTexture(), ax, ay, w, h);
            } else {
                // ── INLINE mode: draw directly to active target ──
                r.EndScreenSpace();
                if (useAutoOffset) r.SetCameraOffset(camWorldX - ax, camWorldY - ay);
                onRender(r, ax, ay, w, h);
                r.BeginScreenSpace();
            }
        } else {
            if (showBg) {
                r.SetDrawColor(10, 10, 14);
                r.FillRect(ax, ay, w, h);
            }
            if (showNoRender) {
                const char* msg = "Viewport2D";
                float mw = f.MeasureW(msg);
                r.SetDrawColor(s.textDim.r, s.textDim.g, s.textDim.b);
                f.DrawText(r, msg, ax + (w - mw) * 0.5f,
                                   ay + (h - f.GlyphH()) * 0.5f);
            }
        }

        // Border on top
        if (showBorder) {
            r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b);
            r.DrawRect(ax, ay, w, h);
        }

        // Overlay children (HUD)
        for (auto& c : children) c->Render(r, f, ax, ay);
    }

    bool ProcessInput(float mx, float my, bool ldown, bool lclick, bool lrelease,
                      TitanUI* ui, float ox, float oy) override
    {
        if (!visible || !enabled) return false;
        float ax = ox+x, ay = oy+y;
        bool inView = mx >= ax && mx < ax+w && my >= ay && my < ay+h;

        // Forward to overlay children
        if (inView) {
            for (int i = (int)children.size()-1; i >= 0; i--) {
                if (children[i]->ProcessInput(mx, my, ldown, lclick, lrelease,
                                              ui, ax, ay))
                    return true;
            }
        }
        return inView && (ldown || lclick);
    }
};

} // namespace Titan
