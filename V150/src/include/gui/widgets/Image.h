// Image.h — Draws a Texture2D inside a rect, with optional tint.
#pragma once
#include "../Widget.h"
#include "../TitanStyle.h"
#include "../../Texture.h"

namespace Titan {

class Image : public Widget {
public:
    LightningEngine::Texture* texture = nullptr;

    // UV region (0-1 normalized). Default = full texture.
    float u0=0.f, v0=0.f, u1=1.f, v1=1.f;

    // Tint colour (white = no tint).
    Uint8 tintR=255, tintG=255, tintB=255, tintA=255;

    Image() = default;
    Image(float x, float y, float w, float h, LightningEngine::Texture* tex = nullptr)
        : texture(tex)
    { this->x=x; this->y=y; this->w=w; this->h=h; }

    void SetUV(float u0_, float v0_, float u1_, float v1_)
    { u0=u0_; v0=v0_; u1=u1_; v1=v1_; }

    void SetTint(Uint8 r, Uint8 g, Uint8 b, Uint8 a=255)
    { tintR=r; tintG=g; tintB=b; tintA=a; }

    void Render(LightningEngine::Renderer& r, TitanFont& /*f*/, float ox, float oy) override
    {
        if (!visible) return;
        float ax = ox+x, ay = oy+y;

        if (texture && texture->IsValid()) {
            r.SetDrawColor(tintR, tintG, tintB, tintA);
            r.DrawTextureRegion(*texture, ax, ay, w, h, u0, v0, u1, v1);
        } else {
            // Placeholder — dim checkerboard-ish rect
            const Style& s = gStyle;
            r.SetDrawColor(s.panelBg.r, s.panelBg.g, s.panelBg.b);
            r.FillRect(ax, ay, w, h);
            r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b);
            r.DrawRect(ax, ay, w, h);
        }
    }
};

} // namespace Titan
