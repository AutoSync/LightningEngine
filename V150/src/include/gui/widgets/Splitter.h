// Splitter.h — Resizable two-panel split layout widget.
// The first two children (added via Add<>()) are panelA and panelB.
#pragma once
#include <functional>
#include <algorithm>
#include "../Widget.h"
#include "../TitanStyle.h"
#include "../TitanFont.h"

namespace Titan {

class Splitter : public Widget {
public:
    bool  vertical  = true;   // true = left/right split, false = top/bottom
    float splitPos  = 0.5f;   // 0..1 relative position of the handle
    float minA      = 50.f;   // minimum size for panelA
    float minB      = 50.f;   // minimum size for panelB
    float handleW   = 5.f;    // handle thickness

    std::function<void(float)> onResize;

    Splitter() = default;
    Splitter(float x, float y, float w, float h, bool vert = true)
        : vertical(vert)
    { this->x=x; this->y=y; this->w=w; this->h=h; }

    bool ProcessInput(float mx, float my, bool ldown, bool lclick, bool lrelease,
                      TitanUI* ui, float ox, float oy) override
    {
        if (!visible || !enabled) return false;
        float ax = ox+x, ay = oy+y;

        float totalSize = vertical ? w : h;
        float handlePos = splitPos * totalSize;

        // Handle rect
        float hx, hy, hw, hh;
        if (vertical) {
            hx = ax + handlePos - handleW * 0.5f;
            hy = ay;
            hw = handleW;
            hh = h;
        } else {
            hx = ax;
            hy = ay + handlePos - handleW * 0.5f;
            hw = w;
            hh = handleW;
        }

        bool inHandle = mx >= hx && mx < hx + hw && my >= hy && my < hy + hh;

        if (lclick && inHandle) {
            dragging   = true;
            dragStart  = vertical ? mx : my;
            dragSplit  = splitPos;
            SetUICapture(ui, this);
        }

        if (dragging) {
            float cur    = vertical ? mx : my;
            float delta  = cur - dragStart;
            float newSplit = dragSplit + delta / totalSize;
            // Clamp to min sizes
            float minSplit = minA / totalSize;
            float maxSplit = 1.f - minB / totalSize;
            if (newSplit < minSplit) newSplit = minSplit;
            if (newSplit > maxSplit) newSplit = maxSplit;
            if (newSplit != splitPos) {
                splitPos = newSplit;
                if (onResize) onResize(splitPos);
            }
            hoverHandle = true;
            if (!ldown) {
                dragging = false;
                SetUICapture(ui, nullptr);
            }
            return true;
        }

        hoverHandle = inHandle;

        // Forward input to child panels
        updateChildLayouts();

        if (children.size() >= 1) {
            if (children[0]->ProcessInput(mx, my, ldown, lclick, lrelease, ui, ax, ay))
                return true;
        }
        if (children.size() >= 2) {
            if (children[1]->ProcessInput(mx, my, ldown, lclick, lrelease, ui, ax, ay))
                return true;
        }

        bool inWidget = mx >= ax && mx < ax+w && my >= ay && my < ay+h;
        return inWidget && (ldown || lclick);
    }

    void Render(LightningEngine::Renderer& r, TitanFont& f, float ox, float oy) override
    {
        if (!visible) return;
        if (children.size() < 2) {
            // Render children if any, no special layout
            for (auto& c : children) c->Render(r, f, ox+x, oy+y);
            return;
        }

        float ax = ox+x, ay = oy+y;
        const Style& s = gStyle;

        updateChildLayouts();

        // Render panelA
        children[0]->Render(r, f, ax, ay);

        // Render panelB
        children[1]->Render(r, f, ax, ay);

        // Handle
        float totalSize = vertical ? w : h;
        float handlePos = splitPos * totalSize;

        float hx, hy, hw, hh;
        if (vertical) {
            hx = ax + handlePos - handleW * 0.5f;
            hy = ay;
            hw = handleW;
            hh = h;
        } else {
            hx = ax;
            hy = ay + handlePos - handleW * 0.5f;
            hw = w;
            hh = handleW;
        }

        // Base handle color (panelBorder); slightly lighter when hovered
        Color hc = s.panelBorder;
        if (hoverHandle || dragging) {
            hc.r = (Uint8)(hc.r + 30 < 255 ? hc.r + 30 : 255);
            hc.g = (Uint8)(hc.g + 30 < 255 ? hc.g + 30 : 255);
            hc.b = (Uint8)(hc.b + 30 < 255 ? hc.b + 30 : 255);
        }
        r.SetDrawColor(hc.r, hc.g, hc.b);
        r.FillRect(hx, hy, hw, hh);

        // Render additional children (index >= 2) normally
        for (int i = 2; i < (int)children.size(); i++)
            children[i]->Render(r, f, ax, ay);
    }

private:
    bool  hoverHandle = false;
    bool  dragging    = false;
    float dragStart   = 0.f;
    float dragSplit   = 0.5f;

    void updateChildLayouts()
    {
        if (children.size() < 2) return;
        float totalSize = vertical ? w : h;
        float aSize     = splitPos * totalSize - handleW * 0.5f;
        float bOffset   = splitPos * totalSize + handleW * 0.5f;
        float bSize     = totalSize - bOffset;

        if (vertical) {
            children[0]->x = 0.f; children[0]->y = 0.f;
            children[0]->w = aSize; children[0]->h = h;
            children[1]->x = bOffset; children[1]->y = 0.f;
            children[1]->w = bSize; children[1]->h = h;
        } else {
            children[0]->x = 0.f; children[0]->y = 0.f;
            children[0]->w = w;   children[0]->h = aSize;
            children[1]->x = 0.f; children[1]->y = bOffset;
            children[1]->w = w;   children[1]->h = bSize;
        }
    }
};

} // namespace Titan
