// List.h — Scrollable, single/multi-select list widget.
// Items are rows of uniform height. A vertical scrollbar appears when
// items exceed the visible area. Supports mouse-wheel scrolling.
#pragma once
#include <vector>
#include <string>
#include <functional>
#include <unordered_set>
#include "../Widget.h"
#include "../TitanStyle.h"
#include "../TitanFont.h"

namespace Titan {

class List : public Widget {
public:
    struct Item {
        std::string text;
        bool        enabled = true;
    };

    std::vector<Item> items;
    int  selectedIndex = -1;
    bool multiSelect   = false;
    std::unordered_set<int> selection;  // used when multiSelect=true

    float itemH       = 0.f;   // 0 = use gStyle.lineH
    float scrollOffset = 0.f;
    int   hoverIndex  = -1;

    static constexpr float kScrollbarW  = 8.f;
    static constexpr float kWheelSpeed  = 1.f; // items per wheel tick

    std::function<void(int, const std::string&)> onSelect;

    List() = default;
    List(float x, float y, float w, float h)
    { this->x=x; this->y=y; this->w=w; this->h=h; }

    void AddItem(const std::string& text, bool enabled = true)
    { items.push_back({text, enabled}); }

    void Clear() { items.clear(); selectedIndex = -1; selection.clear(); scrollOffset = 0.f; }

    const std::string& GetSelected() const {
        static const std::string empty;
        return (selectedIndex >= 0 && selectedIndex < (int)items.size())
               ? items[selectedIndex].text : empty;
    }

    bool ProcessInput(float mx, float my, bool ldown, bool lclick, bool lrelease,
                      TitanUI* ui, float ox, float oy) override
    {
        if (!visible || !enabled) return false;
        float ax = ox+x, ay = oy+y;
        float rowH = itemH > 0.f ? itemH : gStyle.lineH;
        float totalH = rowH * (float)items.size();
        float visH   = h;
        bool  hasBar = totalH > visH;
        float innerW = hasBar ? w - kScrollbarW : w;
        bool  inView = mx >= ax && mx < ax+w && my >= ay && my < ay+h;

        // Mouse wheel
        if (inView && ui && GetUIScrollY(ui) != 0.f) {
            scrollOffset -= GetUIScrollY(ui) * rowH * kWheelSpeed;
            clampScroll(totalH, visH);
        }

        // Scrollbar drag
        if (hasBar) {
            float maxOff  = totalH - visH;
            float thumbH  = visH * (visH / totalH);
            if (thumbH < 12.f) thumbH = 12.f;
            float thumbT  = maxOff > 0.f ? scrollOffset / maxOff : 0.f;
            float thumbY  = ay + thumbT * (visH - thumbH);
            float sbX     = ax + innerW;

            bool inThumb = mx >= sbX && mx < sbX + kScrollbarW
                        && my >= thumbY && my < thumbY + thumbH;
            if (lclick && inThumb) {
                SetUICapture(ui, this);
            }
        }

        // Hover + click on items
        hoverIndex = -1;
        if (inView) {
            float relY = my - ay + scrollOffset;
            int idx = (int)(relY / rowH);
            if (idx >= 0 && idx < (int)items.size()) {
                hoverIndex = idx;
                if (lclick && items[idx].enabled) {
                    if (multiSelect) {
                        if (selection.count(idx)) selection.erase(idx);
                        else selection.insert(idx);
                        selectedIndex = idx;
                    } else {
                        selectedIndex = idx;
                    }
                    if (onSelect) onSelect(idx, items[idx].text);
                    return true;
                }
            }
        }

        return inView && (ldown || lclick);
    }

    void Render(LightningEngine::Renderer& r, TitanFont& f, float ox, float oy) override
    {
        if (!visible) return;
        float ax = ox+x, ay = oy+y;
        const Style& s = gStyle;
        float rowH   = itemH > 0.f ? itemH : s.lineH;
        float totalH = rowH * (float)items.size();
        float visH   = h;
        bool  hasBar = totalH > visH;
        float innerW = hasBar ? w - kScrollbarW : w;

        clampScroll(totalH, visH);

        // Background
        r.SetDrawColor(s.panelBg.r, s.panelBg.g, s.panelBg.b);
        r.FillRect(ax, ay, w, visH);

        // Items
        int firstVisible = (int)(scrollOffset / rowH);
        if (firstVisible < 0) firstVisible = 0;
        float startY = ay - scrollOffset + firstVisible * rowH;

        for (int i = firstVisible; i < (int)items.size(); i++) {
            float iy = startY + (i - firstVisible) * rowH;
            if (iy > ay + visH) break;
            if (iy + rowH < ay) continue;

            const Item& item = items[i];
            bool sel = multiSelect ? (selection.count(i) > 0) : (i == selectedIndex);

            // Row background
            if (sel) {
                r.SetDrawColor(s.highlight.r, s.highlight.g, s.highlight.b);
                r.FillRect(ax, iy, innerW, rowH);
            } else if (i == hoverIndex) {
                r.SetDrawColor(s.selection.r, s.selection.g, s.selection.b, s.selection.a);
                r.FillRect(ax, iy, innerW, rowH);
            }

            // Item text
            Color tc = !item.enabled ? s.textDim
                     : (sel         ? s.textBright
                     :                s.textNormal);
            r.SetDrawColor(tc.r, tc.g, tc.b);
            f.DrawText(r, item.text.c_str(),
                       ax + s.padding,
                       iy + (rowH - f.GlyphH()) * 0.5f);
        }

        // Scrollbar
        if (hasBar) {
            float maxOff = totalH - visH;
            float thumbH = visH * (visH / totalH);
            if (thumbH < 12.f) thumbH = 12.f;
            float thumbT = maxOff > 0.f ? scrollOffset / maxOff : 0.f;
            float thumbY = ay + thumbT * (visH - thumbH);
            float sbX    = ax + innerW;

            r.SetDrawColor(s.sliderTrack.r, s.sliderTrack.g, s.sliderTrack.b);
            r.FillRect(sbX, ay, kScrollbarW, visH);
            r.SetDrawColor(s.sliderThumb.r, s.sliderThumb.g, s.sliderThumb.b);
            r.FillRect(sbX + 1.f, thumbY, kScrollbarW - 2.f, thumbH);
        }

        // Border
        r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b);
        r.DrawRect(ax, ay, w, visH);
    }

private:
    void clampScroll(float totalH, float visH)
    {
        float maxOff = totalH > visH ? totalH - visH : 0.f;
        if (scrollOffset < 0.f)    scrollOffset = 0.f;
        if (scrollOffset > maxOff) scrollOffset = maxOff;
    }
};

} // namespace Titan
