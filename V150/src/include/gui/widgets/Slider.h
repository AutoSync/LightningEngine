// Slider.h — Horizontal float slider widget.
#pragma once
#include <functional>
#include <string>
#include <cstdio>
#include "../Widget.h"
#include "../TitanStyle.h"
#include "../TitanFont.h"

namespace Titan {

class TitanUI; // forward

class Slider : public Widget {
public:
    std::string label;
    float value   = 0.f;
    float minVal  = 0.f;
    float maxVal  = 1.f;
    bool  dragging = false;

    std::function<void(float)> onChange;

    static constexpr float kValueW = 72.f; // pixels reserved for "X -100.00"

    Slider() = default;
    Slider(float x, float y, float w, const char* label,
           float minV, float maxV, float val, float /*ignored*/ = 0.f)
        : label(label ? label : ""), value(val), minVal(minV), maxVal(maxV)
    { this->x=x; this->y=y; this->w=w; this->h=18; }

    bool ProcessInput(float mx, float my, bool ldown, bool lclick, bool lrelease,
                      TitanUI* ui, float ox, float oy) override
    {
        if (!visible || !enabled) return false;
        float ax = ox+x, ay = oy+y;
        float trackX = ax + gStyle.padding;
        float trackW = w - gStyle.padding*2.f - kValueW;

        if (lclick && Contains(mx, my, ox, oy)) {
            dragging = true;
            SetUICapture(ui, this);
        }
        if (dragging) {
            float t = (mx - trackX) / trackW;
            t = t < 0.f ? 0.f : (t > 1.f ? 1.f : t);
            value = minVal + t * (maxVal - minVal);
            if (onChange) onChange(value);
            if (!ldown) { dragging = false; SetUICapture(ui, nullptr); }
            return true;
        }
        return false;
    }

    void Render(LightningEngine::Renderer& r, TitanFont& f, float ox, float oy) override
    {
        if (!visible) return;
        float ax = ox+x, ay = oy+y;
        const Style& s = gStyle;
        float trackX = ax + s.padding;
        float trackH = 5.f;
        float trackY = ay + (h - trackH) * 0.5f;
        float trackW = w - s.padding*2.f - kValueW;
        float t = (maxVal > minVal) ? (value - minVal) / (maxVal - minVal) : 0.f;

        // Track
        r.SetDrawColor(s.sliderTrack.r, s.sliderTrack.g, s.sliderTrack.b);
        r.FillRect(trackX, trackY, trackW, trackH);
        // Fill
        r.SetDrawColor(s.sliderFill.r, s.sliderFill.g, s.sliderFill.b);
        r.FillRect(trackX, trackY, trackW * t, trackH);
        // Thumb
        r.SetDrawColor(s.sliderThumb.r, s.sliderThumb.g, s.sliderThumb.b);
        r.FillRect(trackX + trackW*t - 4.f, trackY - 3.f, 8.f, trackH + 6.f);

        // Label + value
        char buf[64];
        SDL_snprintf(buf, sizeof(buf), "%s %.2f", label.c_str(), value);
        r.SetDrawColor(s.textNormal.r, s.textNormal.g, s.textNormal.b);
        f.DrawText(r, buf, trackX + trackW + s.padding,
                   ay + (h - f.GlyphH()) * 0.5f);
    }
};

} // namespace Titan
