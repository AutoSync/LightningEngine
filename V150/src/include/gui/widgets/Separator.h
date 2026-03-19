// Separator.h — Thin horizontal or vertical divider line.
// Useful inside VStack/HStack as a visual section break.
#pragma once
#include "../Widget.h"
#include "../TitanStyle.h"

namespace Titan {

class Separator : public Widget {
public:
    bool  vertical = false;  // false = horizontal line, true = vertical line
    float thickness = 1.f;

    Separator() = default;

    // Horizontal separator (full-width, fixed height = thickness + margin)
    explicit Separator(float w, float margin = 3.f)
        : vertical(false), thickness(1.f)
    { this->x=0; this->y=0; this->w=w; this->h=thickness + margin * 2.f; }

    // Vertical separator (full-height, fixed width = thickness + margin)
    Separator(float h, bool /*vert*/, float margin = 3.f)
        : vertical(true), thickness(1.f)
    { this->x=0; this->y=0; this->w=thickness + margin * 2.f; this->h=h; }

    void Render(LightningEngine::Renderer& r, TitanFont& /*f*/, float ox, float oy) override
    {
        if (!visible) return;
        float ax = ox+x, ay = oy+y;
        const Style& s = gStyle;
        r.SetDrawColor(s.separator.r, s.separator.g, s.separator.b);

        if (!vertical) {
            float sy = ay + (h - thickness) * 0.5f;
            r.FillRect(ax, sy, w, thickness);
        } else {
            float sx = ax + (w - thickness) * 0.5f;
            r.FillRect(sx, ay, thickness, h);
        }
    }
};

} // namespace Titan
