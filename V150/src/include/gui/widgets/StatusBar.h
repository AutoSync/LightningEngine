// StatusBar.h — Thin status bar with labelled sections (left-aligned or right-aligned).
#pragma once
#include <string>
#include <vector>
#include <functional>
#include "../Widget.h"
#include "../TitanStyle.h"
#include "../TitanFont.h"

namespace Titan {

class StatusBar : public Widget {
public:
    struct Section {
        std::string           text;
        Uint8                 r = 160, g = 160, b = 175;
        bool                  rightAlign = false;
        std::function<void()> onClick;
    };

    std::vector<Section> sections;
    Color bgColor = { 16, 16, 22, 255 };

    StatusBar() = default;
    StatusBar(float x, float y, float w, float h)
    { this->x=x; this->y=y; this->w=w; this->h=h; }

    Section& AddSection(const char* text, Uint8 r=160, Uint8 g=160, Uint8 b=175,
                        bool rightAlign=false)
    {
        sections.push_back({ text ? text : "", r, g, b, rightAlign });
        return sections.back();
    }

    void SetText(int idx, const char* text)
    {
        if (idx >= 0 && idx < (int)sections.size())
            sections[idx].text = text ? text : "";
    }

    bool ProcessInput(float mx, float my, bool ldown, bool lclick, bool lrelease,
                      TitanUI* ui, float ox, float oy) override
    {
        if (!visible || !enabled) return false;
        if (!Contains(mx, my, ox, oy)) return false;
        return ldown || lclick;
    }

    void Render(LightningEngine::Renderer& r, TitanFont& f, float ox, float oy) override
    {
        if (!visible) return;
        float ax  = ox+x, ay = oy+y;
        const Style& s = gStyle;

        r.SetDrawColor(bgColor.r, bgColor.g, bgColor.b);
        r.FillRect(ax, ay, w, h);
        r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b);
        r.FillRect(ax, ay, w, 1.f);   // top border

        float pad   = s.padding;
        float ty    = ay + (h - f.GlyphH()) * 0.5f;
        float leftX = ax + pad;
        float rightX = ax + w - pad;

        for (const auto& sec : sections) {
            if (sec.text.empty()) continue;
            float tw = f.MeasureW(sec.text.c_str());
            float tx;
            if (sec.rightAlign) {
                rightX -= tw;
                tx = rightX;
                rightX -= pad;
            } else {
                tx = leftX;
                leftX += tw + pad * 2.f;
            }
            r.SetDrawColor(sec.r, sec.g, sec.b);
            f.DrawText(r, sec.text.c_str(), tx, ty);
        }
    }
};

} // namespace Titan
