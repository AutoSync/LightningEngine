// TextureViewerWidget.h — Interactive pan/zoom texture viewer for the editor.
//
// Usage:
//   auto* tv = panel->Add<TextureViewerWidget>(x, y, w, h);
//   tv->tex     = &myTexture;      // non-owning reference
//   tv->gridTex = &checkerTex;     // optional checkerboard background
//   tv->ResetView();               // fit-to-panel at first display
//
// Mouse wheel   : zoom in/out, anchored at cursor position
// Left drag     : pan
// Double-click  : reset to fit view
#pragma once
#include <algorithm>
#include <cmath>
#include <string>
#include "../Widget.h"
#include "../TitanStyle.h"
#include "../TitanFont.h"
#include "../../Texture.h"

namespace Titan {

class TextureViewerWidget : public Widget {
public:
    LightningEngine::Texture* tex     = nullptr;  // non-owning
    LightningEngine::Texture* gridTex = nullptr;  // checkerboard, non-owning

    float zoom = 1.f;
    float panX = 0.f;
    float panY = 0.f;

    bool  fitOnFirstRender = true;   // auto-fit the first time Render is called

private:
    bool   hasFit_      = false;
    bool   dragging_    = false;
    float  dragStartX_  = 0.f;
    float  dragStartY_  = 0.f;
    float  dragPanX_    = 0.f;
    float  dragPanY_    = 0.f;
    Uint64 lastClickMs_ = 0;

public:
    TextureViewerWidget() = default;
    TextureViewerWidget(float x, float y, float w, float h)
    { this->x = x; this->y = y; this->w = w; this->h = h; }

    // Fit the texture to the widget, centering it with 5 % padding.
    void ResetView()
    {
        hasFit_ = true;
        if (!tex || !tex->IsValid()) { zoom = 1.f; panX = 0.f; panY = 0.f; return; }
        float tw = static_cast<float>(tex->GetWidth());
        float th = static_cast<float>(tex->GetHeight());
        if (tw <= 0.f || th <= 0.f) return;
        float scaleX = w  / tw;
        float scaleY = h  / th;
        zoom = std::min(scaleX, scaleY) * 0.92f;
        panX = 0.f;
        panY = 0.f;
    }

    bool ProcessInput(float mx, float my, bool ldown, bool lclick, bool lrelease,
                      TitanUI* ui, float ox, float oy) override
    {
        if (!visible || !enabled) return false;
        float ax = ox + x, ay = oy + y;
        bool inView = (mx >= ax && mx < ax + w && my >= ay && my < ay + h);

        // ── Scroll-wheel zoom ────────────────────────────────────────────
        if (inView) {
            float scrollY = GetUIScrollY(ui);
            if (scrollY != 0.f) {
                float newZoom = zoom * (scrollY > 0.f ? 1.12f : (1.f / 1.12f));
                newZoom = std::max(0.01f, std::min(newZoom, 128.f));
                // Anchor zoom at cursor
                float cx = ax + w * 0.5f + panX;
                float cy = ay + h * 0.5f + panY;
                float relX = mx - cx;
                float relY = my - cy;
                float scale = newZoom / zoom;
                panX -= relX * (scale - 1.f);
                panY -= relY * (scale - 1.f);
                zoom = newZoom;
            }
        }

        // ── Double-click → reset view ────────────────────────────────────
        if (lclick && inView) {
            Uint64 now = SDL_GetTicks();
            if (now - lastClickMs_ <= 350ULL) {
                ResetView();
                lastClickMs_ = 0;
            } else {
                lastClickMs_ = now;
            }
        }

        // ── Left-drag pan ────────────────────────────────────────────────
        if (lclick && inView) {
            dragging_   = true;
            dragStartX_ = mx;
            dragStartY_ = my;
            dragPanX_   = panX;
            dragPanY_   = panY;
            SetUICapture(ui, this);
        }
        if (dragging_) {
            panX = dragPanX_ + (mx - dragStartX_);
            panY = dragPanY_ + (my - dragStartY_);
            if (lrelease || !ldown) {
                dragging_ = false;
                SetUICapture(ui, nullptr);
            }
            return true;
        }

        return inView && ldown;
    }

    void Render(LightningEngine::Renderer& r, TitanFont& f, float ox, float oy) override
    {
        if (!visible) return;
        float ax = ox + x, ay = oy + y;
        const Style& s = gStyle;

        // ── Background ───────────────────────────────────────────────────
        r.SetDrawColor(22, 22, 24);
        r.FillRect(ax, ay, w, h);

        // Auto-fit on first display
        if (fitOnFirstRender && !hasFit_) ResetView();

        if (!tex || !tex->IsValid()) {
            r.SetDrawColor(s.textDim.r, s.textDim.g, s.textDim.b);
            const char* msg = "Sem textura carregada";
            float mw = f.MeasureW(msg);
            f.DrawText(r, msg, ax + (w - mw) * 0.5f, ay + (h - f.GlyphH()) * 0.5f);
            r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b);
            r.DrawRect(ax, ay, w, h);
            return;
        }

        float tw = static_cast<float>(tex->GetWidth());
        float th = static_cast<float>(tex->GetHeight());
        float dispW = tw * zoom;
        float dispH = th * zoom;
        float imgX  = ax + w * 0.5f + panX - dispW * 0.5f;
        float imgY  = ay + h * 0.5f + panY - dispH * 0.5f;

        // ── Checkerboard background inside the image rect ─────────────────
        if (gridTex && gridTex->IsValid() && dispW > 0.f && dispH > 0.f) {
            const float cellSz = 16.f;
            float tilesU = dispW / cellSz;
            float tilesV = dispH / cellSz;
            r.SetDrawColor(255, 255, 255);
            r.DrawTextureRegion(*gridTex, imgX, imgY, dispW, dispH,
                                0.f, 0.f, tilesU, tilesV);
        }

        // ── The texture itself ────────────────────────────────────────────
        if (dispW > 0.f && dispH > 0.f) {
            r.SetDrawColor(255, 255, 255);
            r.DrawTexture(*tex, imgX, imgY, dispW, dispH);
        }

        // ── Zoom label ────────────────────────────────────────────────────
        char zoomBuf[32];
        SDL_snprintf(zoomBuf, sizeof(zoomBuf), "%.0f%%", zoom * 100.f);
        float lw = f.MeasureW(zoomBuf);
        r.SetDrawColor(30, 30, 30, 180);
        r.FillRect(ax + w - lw - 10.f, ay + h - f.GlyphH() - 6.f, lw + 8.f, f.GlyphH() + 4.f);
        r.SetDrawColor(s.textBright.r, s.textBright.g, s.textBright.b);
        f.DrawText(r, zoomBuf, ax + w - lw - 6.f, ay + h - f.GlyphH() - 4.f);

        // ── Border ───────────────────────────────────────────────────────
        r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b);
        r.DrawRect(ax, ay, w, h);
    }
};

} // namespace Titan
