// Border.h — Decorator widget that draws a colored border around its children.
// Acts as a transparent container: children are rendered inside the padded area.
// thickness controls the border width; padding adds space between border and children.
//
// Usage:
//   auto* b = panel->Add<Border>(x, y, w, h, 1.f, 4.f);   // 1px border, 4px padding
//   b->Add<Button>(...);
#pragma once
#include "../Widget.h"
#include "../TitanStyle.h"
#include "../TitanFont.h"

namespace Titan {

class Border : public Widget {
public:
    float thickness = 1.f;
    float innerPad  = 0.f;   // extra padding between border edge and children

    // Border colour. If useStyleColor=true, uses panelBorder from gStyle.
    Uint8 R=0, G=0, B=0, A=255;
    bool  useStyleColor = true;

    // Optional background fill inside the border.
    bool  fillBg = false;
    Uint8 bgR=0, bgG=0, bgB=0, bgA=255;

    Border() = default;
    Border(float x, float y, float w, float h,
           float thickness = 1.f, float innerPad = 0.f)
        : thickness(thickness), innerPad(innerPad)
    { this->x=x; this->y=y; this->w=w; this->h=h; }

    void SetBorderColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255)
    { R=r; G=g; B=b; A=a; useStyleColor=false; }

    void SetBackground(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255)
    { bgR=r; bgG=g; bgB=b; bgA=a; fillBg=true; }

    bool ProcessInput(float mx, float my, bool ldown, bool lclick, bool lrelease,
                      TitanUI* ui, float ox, float oy) override
    {
        if (!visible || !enabled) return false;
        float ax = ox+x, ay = oy+y;
        float off = thickness + innerPad;
        for (int i = (int)children.size()-1; i >= 0; i--) {
            if (children[i]->ProcessInput(mx, my, ldown, lclick, lrelease, ui,
                                          ax + off, ay + off))
                return true;
        }
        return false;
    }

    void Render(LightningEngine::Renderer& r, TitanFont& f, float ox, float oy) override
    {
        if (!visible) return;
        float ax = ox+x, ay = oy+y;
        const Style& s = gStyle;
        float off = thickness + innerPad;

        // Optional background
        if (fillBg) {
            r.SetDrawColor(bgR, bgG, bgB, bgA);
            r.FillRect(ax + thickness, ay + thickness,
                       w - thickness * 2.f,
                       h - thickness * 2.f);
        }

        // Draw border lines (one rect per side, thickness-wide)
        Color bc = useStyleColor ? s.panelBorder : Color{R, G, B, A};
        r.SetDrawColor(bc.r, bc.g, bc.b, bc.a);
        for (float t = 0.f; t < thickness; t += 1.f) {
            r.DrawRect(ax + t, ay + t, w - t * 2.f, h - t * 2.f);
        }

        // Children positioned inside the border+padding
        for (auto& c : children) c->Render(r, f, ax + off, ay + off);
    }
};

} // namespace Titan
