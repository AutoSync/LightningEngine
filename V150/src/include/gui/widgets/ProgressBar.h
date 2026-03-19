// ProgressBar.h — Horizontal progress bar with optional label.
#pragma once
#include <string>
#include <cstdio>
#include "../Widget.h"
#include "../TitanStyle.h"
#include "../TitanFont.h"

namespace Titan {

class ProgressBar : public Widget {
public:
    float value   = 0.f;   // 0..1
    std::string label;     // optional text shown in the centre
    bool  showPercent = true;

    // Colours (default = style colours)
    Color trackColor;
    Color fillColor;
    Color textColor;
    bool  useStyleColors = true;

    ProgressBar() = default;
    ProgressBar(float x, float y, float w, float h,
                float value = 0.f, const char* label = "")
        : value(value), label(label ? label : "")
    { this->x=x; this->y=y; this->w=w; this->h=h; }

    // Clamp value to [0,1]
    void SetValue(float v) { value = v < 0.f ? 0.f : (v > 1.f ? 1.f : v); }

    void Render(LightningEngine::Renderer& r, TitanFont& f, float ox, float oy) override
    {
        if (!visible) return;
        float ax = ox+x, ay = oy+y;
        const Style& s = gStyle;

        // Track
        Color tc = useStyleColors ? s.sliderTrack : trackColor;
        r.SetDrawColor(tc.r, tc.g, tc.b);
        r.FillRect(ax, ay, w, h);

        // Fill
        float fillW = w * (value < 0.f ? 0.f : (value > 1.f ? 1.f : value));
        if (fillW > 0.f) {
            Color fc = useStyleColors ? s.sliderFill : fillColor;
            r.SetDrawColor(fc.r, fc.g, fc.b);
            r.FillRect(ax, ay, fillW, h);
        }

        // Border
        r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b);
        r.DrawRect(ax, ay, w, h);

        // Centre text
        std::string displayText;
        if (!label.empty()) {
            displayText = label;
            if (showPercent) {
                char buf[16];
                SDL_snprintf(buf, sizeof(buf), " %.0f%%", value * 100.f);
                displayText += buf;
            }
        } else if (showPercent) {
            char buf[16];
            SDL_snprintf(buf, sizeof(buf), "%.0f%%", value * 100.f);
            displayText = buf;
        }

        if (!displayText.empty()) {
            float tw = f.MeasureW(displayText.c_str());
            float tx = ax + (w - tw) * 0.5f;
            float ty = ay + (h - f.GlyphH()) * 0.5f;
            Color xtc = useStyleColors ? s.textBright : textColor;
            r.SetDrawColor(xtc.r, xtc.g, xtc.b);
            f.DrawText(r, displayText.c_str(), tx, ty);
        }
    }
};

} // namespace Titan
