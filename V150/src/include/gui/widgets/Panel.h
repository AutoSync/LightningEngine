// Panel.h — Container widget with optional title bar.
#pragma once
#include "../Widget.h"
#include "../TitanStyle.h"
#include "../TitanFont.h"

namespace Titan {

class Panel : public Widget {
public:
    std::string title;
    bool showTitle = true;

    Panel() = default;
    Panel(float x, float y, float w, float h,
          const char* title = "", bool showTitle = true)
        : title(title ? title : ""), showTitle(showTitle)
    { this->x=x; this->y=y; this->w=w; this->h=h; }

    void Render(LightningEngine::Renderer& r, TitanFont& f, float ox, float oy) override
    {
        if (!visible) return;
        float ax = ox+x, ay = oy+y;
        const Style& s = gStyle;

        // Background
        r.SetDrawColor(s.panelBg.r, s.panelBg.g, s.panelBg.b, s.panelBg.a);
        r.FillRect(ax, ay, w, h);

        // Title bar
        if (showTitle && !title.empty()) {
            r.SetDrawColor(s.panelHeader.r, s.panelHeader.g, s.panelHeader.b);
            r.FillRect(ax, ay, w, s.titleH);
            r.SetDrawColor(s.textBright.r, s.textBright.g, s.textBright.b);
            float ty = ay + (s.titleH - f.GlyphH()) * 0.5f;
            f.DrawText(r, title.c_str(), ax + s.padding, ty);
        }

        // Border
        r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b);
        r.DrawRect(ax, ay, w, h);

        // Children
        for (auto& c : children) c->Render(r, f, ax, ay);
    }

    bool ProcessInput(float mx, float my, bool ldown, bool lclick, bool lrelease,
                      TitanUI* ui, float ox, float oy) override
    {
        if (!visible || !enabled) return false;
        float ax = ox+x, ay = oy+y;
        for (int i = (int)children.size()-1; i >= 0; i--) {
            if (children[i]->ProcessInput(mx, my, ldown, lclick, lrelease, ui, ax, ay))
                return true;
        }
        if (lclick && Contains(mx, my, ox, oy)) return true;
        return false;
    }
};

} // namespace Titan
