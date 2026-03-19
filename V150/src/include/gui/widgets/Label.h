// Label.h — Static text widget.
#pragma once
#include "../Widget.h"
#include "../TitanStyle.h"
#include "../TitanFont.h"

namespace Titan {

class Label : public Widget {
public:
    std::string text;
    Uint8 R=0, G=0, B=0;
    bool  useStyleColor = true;

    Label() = default;
    Label(float x, float y, const char* text)
        : text(text ? text : "")
    { this->x=x; this->y=y; this->w=0; this->h=0; }

    // Legacy constructor — extra params ignored, kept for source compatibility
    Label(float x, float y, const char* text, float /*ignored_scale*/)
        : text(text ? text : "")
    { this->x=x; this->y=y; this->w=0; this->h=0; }

    void SetColor(Uint8 r, Uint8 g, Uint8 b) { R=r; G=g; B=b; useStyleColor=false; }
    void SetText(const char* t) { text = t ? t : ""; }

    void Render(LightningEngine::Renderer& r, TitanFont& f, float ox, float oy) override
    {
        if (!visible || text.empty()) return;
        const Color& c = useStyleColor ? gStyle.textNormal : Color{R,G,B};
        r.SetDrawColor(c.r, c.g, c.b);
        f.DrawText(r, text.c_str(), ox+x, oy+y);
    }
};

} // namespace Titan
