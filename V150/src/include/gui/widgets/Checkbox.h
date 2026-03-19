// Checkbox.h — Toggle checkbox widget.
#pragma once
#include <functional>
#include "../Widget.h"
#include "../TitanStyle.h"
#include "../TitanFont.h"

namespace Titan {

class Checkbox : public Widget {
public:
    std::string text;
    bool checked = false;
    bool hover   = false;

    std::function<void(bool)> onChange;

    Checkbox() = default;
    Checkbox(float x, float y, const char* text, bool checked = false, float /*ignored*/ = 0.f)
        : text(text ? text : ""), checked(checked)
    { this->x=x; this->y=y; this->w=200; this->h=18; }

    bool ProcessInput(float mx, float my, bool ldown, bool lclick, bool lrelease,
                      TitanUI* ui, float ox, float oy) override
    {
        if (!visible || !enabled) return false;
        hover = Contains(mx, my, ox, oy);
        if (hover && lclick) {
            checked = !checked;
            if (onChange) onChange(checked);
            return true;
        }
        return hover && ldown;
    }

    void Render(LightningEngine::Renderer& r, TitanFont& f, float ox, float oy) override
    {
        if (!visible) return;
        float ax = ox+x, ay = oy+y;
        const Style& s = gStyle;
        float boxSz = f.GlyphH();

        r.SetDrawColor(s.sliderTrack.r, s.sliderTrack.g, s.sliderTrack.b);
        r.FillRect(ax, ay, boxSz, boxSz);
        r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b);
        r.DrawRect(ax, ay, boxSz, boxSz);

        if (checked) {
            r.SetDrawColor(s.checkMark.r, s.checkMark.g, s.checkMark.b);
            float m = 2.f;
            r.FillRect(ax+m, ay+m, boxSz-m*2, boxSz-m*2);
        }

        r.SetDrawColor(hover ? s.textBright.r : s.textNormal.r,
                       hover ? s.textBright.g : s.textNormal.g,
                       hover ? s.textBright.b : s.textNormal.b);
        f.DrawText(r, text.c_str(), ax + boxSz + s.padding, ay + (boxSz - f.GlyphH())*0.5f);
    }
};

} // namespace Titan
