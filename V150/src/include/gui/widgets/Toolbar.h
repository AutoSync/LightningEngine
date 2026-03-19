// Toolbar.h — Horizontal toolbar strip with buttons and separators.
#pragma once
#include <vector>
#include <string>
#include <functional>
#include "../Widget.h"
#include "../TitanStyle.h"
#include "../TitanFont.h"

namespace Titan {

struct ToolItem {
    std::string              label;
    bool                     separator = false;
    bool                     toggle    = false;
    bool                     active    = false;
    float                    bw        = 0.f;  // computed button width
    std::function<void(bool)> onClick;
};

class Toolbar : public Widget {
public:
    std::vector<ToolItem> items;
    int   hoverIdx = -1;
    float padding  = 2.f;

    Toolbar() = default;
    Toolbar(float x, float y, float w, float h)
    { this->x=x; this->y=y; this->w=w; this->h=h; }

    void AddButton(const std::string& label,
                   std::function<void(bool)> cb,
                   bool toggle = false)
    {
        ToolItem it;
        it.label  = label;
        it.toggle = toggle;
        it.active = false;
        it.onClick = cb;
        items.push_back(std::move(it));
    }

    void AddSeparator()
    {
        ToolItem sep;
        sep.separator = true;
        items.push_back(std::move(sep));
    }

    bool ProcessInput(float mx, float my, bool ldown, bool lclick, bool lrelease,
                      TitanUI* ui, float ox, float oy) override
    {
        if (!visible || !enabled) return false;
        float ax = ox+x, ay = oy+y;
        bool inBar = mx >= ax && mx < ax+w && my >= ay && my < ay+h;

        hoverIdx = -1;
        if (!inBar) return false;

        // Layout items to find hover/click target
        float cx = ax + padding;
        for (int i = 0; i < (int)items.size(); i++) {
            ToolItem& it = items[i];
            if (it.separator) {
                cx += 6.f; // separator width
                continue;
            }
            float bw = itemWidth(it);
            if (mx >= cx && mx < cx + bw) {
                hoverIdx = i;
                if (lclick) {
                    if (it.toggle) it.active = !it.active;
                    if (it.onClick) it.onClick(it.active);
                    return true;
                }
                break;
            }
            cx += bw + padding;
        }

        return inBar && (ldown || lclick);
    }

    void Render(LightningEngine::Renderer& r, TitanFont& f, float ox, float oy) override
    {
        if (!visible) return;
        float ax = ox+x, ay = oy+y;
        const Style& s = gStyle;

        // Bar background
        r.SetDrawColor(s.panelBg.r, s.panelBg.g, s.panelBg.b);
        r.FillRect(ax, ay, w, h);

        float cx = ax + padding;
        for (int i = 0; i < (int)items.size(); i++) {
            ToolItem& it = items[i];

            if (it.separator) {
                // Thin vertical line
                r.SetDrawColor(s.separator.r, s.separator.g, s.separator.b);
                r.FillRect(cx + 2.f, ay + padding, 1.f, h - padding * 2.f);
                cx += 6.f;
                continue;
            }

            float bw = itemWidth(it, &f);
            it.bw    = bw;

            // Button background
            Color bg;
            if (it.active)       bg = s.btnActive;
            else if (i==hoverIdx) bg = s.btnHover;
            else                  bg = s.btnNormal;

            r.SetDrawColor(bg.r, bg.g, bg.b);
            r.FillRect(cx, ay + padding, bw, h - padding * 2.f);

            // Label
            float lw = f.MeasureW(it.label.c_str());
            float lx = cx + (bw - lw) * 0.5f;
            float ly = ay + padding + ((h - padding*2.f) - f.GlyphH()) * 0.5f;
            r.SetDrawColor(s.btnText.r, s.btnText.g, s.btnText.b);
            f.DrawText(r, it.label.c_str(), lx, ly);

            cx += bw + padding;
        }

        // Outer border
        r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b);
        r.DrawRect(ax, ay, w, h);
    }

private:
    float itemWidth(const ToolItem& it, TitanFont* f = nullptr) const
    {
        float innerH = h - padding * 2.f;
        if (f) {
            float lw = f->MeasureW(it.label.c_str());
            float bw = lw + padding * 2.f;
            return bw > innerH ? bw : innerH; // at least square
        }
        // Fallback (no font yet)
        float charW = 8.f;
        float lw    = it.label.size() * charW + padding * 2.f;
        return lw > innerH ? lw : innerH;
    }
};

} // namespace Titan
