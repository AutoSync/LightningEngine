// Button.h — Clickable button widget.
#pragma once
#include <functional>
#include "../Widget.h"
#include "../TitanStyle.h"
#include "../TitanFont.h"

namespace Titan {

class Button : public Widget {
public:
    std::string text;
    bool hover  = false;
    bool active = false;

    // Optional text color override (overrides style when useCustomColor=true)
    Uint8 textR = 0, textG = 0, textB = 0;
    bool  useCustomColor = false;

    void SetColor(Uint8 r, Uint8 g, Uint8 b)
    { textR = r; textG = g; textB = b; useCustomColor = true; }

    std::function<void()> onClick;

    Button() = default;
    Button(float x, float y, float w, float h, const char* text, float /*ignored*/ = 0.f)
        : text(text ? text : "")
    { this->x=x; this->y=y; this->w=w; this->h=h; }

    bool ProcessInput(float mx, float my, bool ldown, bool lclick, bool lrelease,
                      TitanUI* ui, float ox, float oy) override
    {
        if (!visible || !enabled) return false;
        bool inside = Contains(mx, my, ox, oy);
        hover  = inside;
        active = inside && ldown;
        if (inside && lclick && onClick) onClick();
        return inside && (ldown || lclick);
    }

    void Render(LightningEngine::Renderer& r, TitanFont& f, float ox, float oy) override
    {
        if (!visible) return;
        float ax = ox+x, ay = oy+y;
        const Style& s = gStyle;

        Color bg = active ? s.btnActive : (hover ? s.btnHover : s.btnNormal);
        r.SetDrawColor(bg.r, bg.g, bg.b);
        r.FillRect(ax, ay, w, h);

        r.SetDrawColor(s.btnBorder.r, s.btnBorder.g, s.btnBorder.b);
        r.DrawRect(ax, ay, w, h);

        if (!text.empty()) {
            float tw = f.MeasureW(text.c_str());
            float tx = ax + (w - tw) * 0.5f;
            float ty = ay + (h - f.GlyphH()) * 0.5f;
            if (useCustomColor)
                r.SetDrawColor(textR, textG, textB);
            else
                r.SetDrawColor(s.btnText.r, s.btnText.g, s.btnText.b);
            f.DrawText(r, text.c_str(), tx, ty);
        }
    }
};

} // namespace Titan
