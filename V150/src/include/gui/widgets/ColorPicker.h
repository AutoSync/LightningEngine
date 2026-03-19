// ColorPicker.h — RGBA color picker with per-channel sliders and a swatch.
#pragma once
#include <functional>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <SDL3/SDL.h>
#include "../Widget.h"
#include "../TitanStyle.h"
#include "../TitanFont.h"

namespace Titan {

class ColorPicker : public Widget {
public:
    float R = 1.f, G = 1.f, B = 1.f, A = 1.f;
    bool  showAlpha = true;

    std::function<void(float,float,float,float)> onChanged;

    static constexpr float kSwatchH  = 28.f;
    static constexpr float kSliderH  = 16.f;
    static constexpr float kLabelW   = 14.f;
    static constexpr float kHexH     = 16.f;
    static constexpr float kRowGap   = 3.f;

    ColorPicker() = default;
    ColorPicker(float x, float y, float w, float h)
    { this->x=x; this->y=y; this->w=w; this->h=h; }

    void SetColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255)
    {
        R = r / 255.f;
        G = g / 255.f;
        B = b / 255.f;
        A = a / 255.f;
    }

    void GetColorBytes(Uint8& r, Uint8& g, Uint8& b, Uint8& a) const
    {
        auto clamp = [](float v) -> Uint8 {
            int i = (int)(v * 255.f + 0.5f);
            if (i < 0)   i = 0;
            if (i > 255) i = 255;
            return (Uint8)i;
        };
        r = clamp(R); g = clamp(G); b = clamp(B); a = clamp(A);
    }

    bool ProcessInput(float mx, float my, bool ldown, bool lclick, bool lrelease,
                      TitanUI* ui, float ox, float oy) override
    {
        if (!visible || !enabled) return false;
        float ax = ox+x, ay = oy+y;
        bool inWidget = mx >= ax && mx < ax+w && my >= ay && my < ay+h;

        int numChan = showAlpha ? 4 : 3;

        if (lclick && inWidget) {
            // Determine which slider row was clicked
            dragChan = hitTestChannel(mx, my, ax, ay, numChan);
            if (dragChan >= 0) {
                dragging = true;
                SetUICapture(ui, this);
                updateChannel(mx, ax, dragChan);
                return true;
            }
        }

        if (dragging) {
            if (ldown) {
                updateChannel(mx, ax, dragChan);
            } else {
                dragging = false;
                dragChan = -1;
                SetUICapture(ui, nullptr);
            }
            return true;
        }

        return inWidget && (ldown || lclick);
    }

    void Render(LightningEngine::Renderer& r, TitanFont& f, float ox, float oy) override
    {
        if (!visible) return;
        float ax = ox+x, ay = oy+y;
        const Style& s = gStyle;

        int numChan = showAlpha ? 4 : 3;

        // -- Swatch --
        Uint8 sr, sg, sb, sa;
        GetColorBytes(sr, sg, sb, sa);

        // Checkerboard background for alpha
        if (showAlpha && sa < 255) {
            r.SetDrawColor(160, 160, 160);
            r.FillRect(ax, ay, w, kSwatchH);
            r.SetDrawColor(220, 220, 220);
            float cs = 8.f;
            for (float gy = ay; gy < ay + kSwatchH; gy += cs)
                for (float gx = ax; gx < ax + w; gx += cs)
                    if ((int)((gx-ax)/cs + (gy-ay)/cs) % 2 == 0)
                        r.FillRect(gx, gy, cs, cs);
        }
        r.SetDrawColor(sr, sg, sb, sa);
        r.FillRect(ax, ay, w, kSwatchH);

        // Swatch border
        r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b);
        r.DrawRect(ax, ay, w, kSwatchH);

        // -- Channel sliders --
        static const char* labels[4] = { "R", "G", "B", "A" };
        float curY = ay + kSwatchH + kRowGap;

        for (int c = 0; c < numChan; c++) {
            float val     = channelValue(c);
            float trackX  = ax + kLabelW + s.padding;
            float trackW  = w - kLabelW - s.padding * 2.f;
            float trackH  = 5.f;
            float trackY  = curY + (kSliderH - trackH) * 0.5f;

            // Label
            r.SetDrawColor(s.textDim.r, s.textDim.g, s.textDim.b);
            f.DrawText(r, labels[c], ax, curY + (kSliderH - f.GlyphH()) * 0.5f);

            // Track background
            r.SetDrawColor(s.sliderTrack.r, s.sliderTrack.g, s.sliderTrack.b);
            r.FillRect(trackX, trackY, trackW, trackH);

            // Fill (tinted with the channel color)
            Uint8 fillR = (c==0) ? 220 : 50;
            Uint8 fillG = (c==1) ? 220 : 50;
            Uint8 fillB = (c==2) ? 220 : (c==3 ? 200 : 50);
            r.SetDrawColor(fillR, fillG, fillB);
            r.FillRect(trackX, trackY, trackW * val, trackH);

            // Thumb
            r.SetDrawColor(s.sliderThumb.r, s.sliderThumb.g, s.sliderThumb.b);
            r.FillRect(trackX + trackW * val - 4.f, trackY - 3.f, 8.f, trackH + 6.f);

            curY += kSliderH + kRowGap;
        }

        // -- Hex display --
        char hexBuf[16];
        SDL_snprintf(hexBuf, sizeof(hexBuf), "#%02X%02X%02X%02X", sr, sg, sb, sa);
        r.SetDrawColor(s.textDim.r, s.textDim.g, s.textDim.b);
        f.DrawText(r, hexBuf, ax + s.padding, curY + (kHexH - f.GlyphH()) * 0.5f);
    }

private:
    int  dragChan = -1;
    bool dragging = false;

    float channelValue(int c) const
    {
        switch(c) {
            case 0: return R;
            case 1: return G;
            case 2: return B;
            case 3: return A;
        }
        return 0.f;
    }

    void setChannel(int c, float v)
    {
        if (v < 0.f) v = 0.f;
        if (v > 1.f) v = 1.f;
        switch(c) {
            case 0: R = v; break;
            case 1: G = v; break;
            case 2: B = v; break;
            case 3: A = v; break;
        }
        if (onChanged) onChanged(R, G, B, A);
    }

    void updateChannel(float mx, float ax, int c)
    {
        float trackX = ax + kLabelW + gStyle.padding;
        float trackW = w - kLabelW - gStyle.padding * 2.f;
        float t = (mx - trackX) / trackW;
        setChannel(c, t);
    }

    // Returns channel index (0-3) if mx,my hits a slider track row, else -1
    int hitTestChannel(float mx, float my, float ax, float ay, int numChan) const
    {
        float curY = ay + kSwatchH + kRowGap;
        for (int c = 0; c < numChan; c++) {
            float trackX = ax + kLabelW + gStyle.padding;
            float trackW = w - kLabelW - gStyle.padding * 2.f;
            float rowBot = curY + kSliderH;
            if (my >= curY && my < rowBot &&
                mx >= trackX - 4.f && mx < trackX + trackW + 4.f)
                return c;
            curY += kSliderH + kRowGap;
        }
        return -1;
    }
};

} // namespace Titan
