// VStack.h — Vertical layout container.
// Positions children top-to-bottom in order.
// Each child keeps its own x/w unless stretchW=true,
// in which case w is overridden to fill the stack width.
// Call Layout() after adding children (or it runs automatically on Render/ProcessInput).
#pragma once
#include "../Widget.h"
#include "../TitanStyle.h"
#include "../TitanFont.h"

namespace Titan {

class VStack : public Widget {
public:
    float spacing   = 4.f;   // gap between children
    float padX      = 0.f;   // left/right inner padding
    float padY      = 0.f;   // top/bottom inner padding
    bool  stretchW  = true;  // override each child's w to fill stack width
    bool  autoH     = true;  // grow h to fit all children

    bool  showBg    = false; // draw background rect (like a Panel)
    bool  dirty     = true;  // layout needs recomputing

    VStack() = default;
    VStack(float x, float y, float w,
           float spacing = 4.f, float padX = 0.f, float padY = 0.f)
        : spacing(spacing), padX(padX), padY(padY)
    { this->x=x; this->y=y; this->w=w; this->h=0; }

    // Force layout recompute next frame
    void MarkDirty() { dirty = true; }

    void Layout()
    {
        if (!dirty) return;
        dirty = false;

        float cx = padX;
        float cy = padY;
        float cw = w - padX * 2.f;

        for (auto& child : children) {
            if (!child->visible) continue;
            child->x = cx;
            child->y = cy;
            if (stretchW) child->w = cw;
            cy += child->h + spacing;
        }

        if (autoH) h = cy - spacing + padY; // shrink last gap
    }

    bool ProcessInput(float mx, float my, bool ldown, bool lclick, bool lrelease,
                      TitanUI* ui, float ox, float oy) override
    {
        if (!visible || !enabled) return false;
        Layout();
        float ax = ox+x, ay = oy+y;
        for (int i = (int)children.size()-1; i >= 0; i--) {
            if (children[i]->ProcessInput(mx, my, ldown, lclick, lrelease, ui, ax, ay))
                return true;
        }
        return false;
    }

    void Render(LightningEngine::Renderer& r, TitanFont& f, float ox, float oy) override
    {
        if (!visible) return;
        Layout();
        float ax = ox+x, ay = oy+y;

        if (showBg) {
            const Style& s = gStyle;
            r.SetDrawColor(s.panelBg.r, s.panelBg.g, s.panelBg.b);
            r.FillRect(ax, ay, w, h);
        }

        for (auto& child : children) child->Render(r, f, ax, ay);
    }
};

} // namespace Titan
