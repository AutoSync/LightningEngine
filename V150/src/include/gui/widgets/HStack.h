// HStack.h — Horizontal layout container.
// Positions children left-to-right in order.
// Each child keeps its own y/h unless stretchH=true.
// Supports equal-width distribution (equalW=true) or natural widths.
#pragma once
#include "../Widget.h"
#include "../TitanStyle.h"
#include "../TitanFont.h"

namespace Titan {

class HStack : public Widget {
public:
    float spacing  = 4.f;  // gap between children
    float padX     = 0.f;  // left/right inner padding
    float padY     = 0.f;  // top/bottom inner padding
    bool  stretchH = true; // override each child's h to fill stack height
    bool  equalW   = false;// divide available width equally among children
    bool  autoW    = false;// grow w to fit all children (overrides equalW)

    bool  showBg   = false;
    bool  dirty    = true;

    HStack() = default;
    HStack(float x, float y, float w, float h,
           float spacing = 4.f, float padX = 0.f, float padY = 0.f)
        : spacing(spacing), padX(padX), padY(padY)
    { this->x=x; this->y=y; this->w=w; this->h=h; }

    void MarkDirty() { dirty = true; }

    void Layout()
    {
        if (!dirty) return;
        dirty = false;

        float cy = padY;
        float ch = h - padY * 2.f;
        float cx = padX;

        int visCount = 0;
        for (auto& c : children) if (c->visible) visCount++;

        float cellW = 0.f;
        if (equalW && visCount > 0) {
            float totalSpacing = spacing * (float)(visCount - 1);
            cellW = (w - padX * 2.f - totalSpacing) / (float)visCount;
        }

        for (auto& child : children) {
            if (!child->visible) continue;
            child->x = cx;
            child->y = cy;
            if (stretchH) child->h = ch;
            if (equalW)   child->w = cellW;
            cx += child->w + spacing;
        }

        if (autoW) w = cx - spacing + padX;
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
