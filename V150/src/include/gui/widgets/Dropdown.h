// Dropdown.h — Single-select dropdown widget.
// Clicking the header opens/closes a popup list of items.
// Items are rendered on top of all other widgets (drawn last via TitanUI root order).
#pragma once
#include <vector>
#include <string>
#include <functional>
#include <cstdio>
#include "../Widget.h"
#include "../TitanStyle.h"
#include "../TitanFont.h"

namespace Titan {

class Dropdown : public Widget {
public:
    std::vector<std::string> items;
    int  selectedIndex = -1;  // -1 = nothing selected
    bool isOpen        = false;
    bool hoverHeader   = false;
    int  hoverItem     = -1;

    std::string placeholder = "Select...";
    static constexpr float kArrowW  = 16.f;
    static constexpr float kItemH   = 18.f;

    std::function<void(int, const std::string&)> onSelect;

    Dropdown() = default;
    Dropdown(float x, float y, float w, float h,
             const char* placeholder = "Select...")
        : placeholder(placeholder ? placeholder : "Select...")
    { this->x=x; this->y=y; this->w=w; this->h=h; }

    void AddItem(const std::string& item) { items.push_back(item); }

    void SetSelected(int idx)
    {
        if (idx >= 0 && idx < (int)items.size()) selectedIndex = idx;
    }

    const std::string& GetSelected() const
    {
        static const std::string empty;
        return (selectedIndex >= 0 && selectedIndex < (int)items.size())
               ? items[selectedIndex] : empty;
    }

    bool ProcessInput(float mx, float my, bool ldown, bool lclick, bool lrelease,
                      TitanUI* ui, float ox, float oy) override
    {
        if (!visible || !enabled) return false;
        float ax = ox+x, ay = oy+y;

        hoverHeader = mx >= ax && mx < ax+w && my >= ay && my < ay+h;

        // Toggle open/close on header click
        if (hoverHeader && lclick) {
            isOpen = !isOpen;
            hoverItem = -1;
            return true;
        }

        // Popup interaction
        if (isOpen) {
            float popY = ay + h;
            float popH = kItemH * (float)items.size();

            bool inPopup = mx >= ax && mx < ax+w && my >= popY && my < popY + popH;

            if (inPopup) {
                hoverItem = (int)((my - popY) / kItemH);
                if (hoverItem < 0) hoverItem = 0;
                if (hoverItem >= (int)items.size()) hoverItem = (int)items.size()-1;

                if (lclick) {
                    selectedIndex = hoverItem;
                    isOpen = false;
                    hoverItem = -1;
                    if (onSelect) onSelect(selectedIndex, items[selectedIndex]);
                    return true;
                }
                return true; // consume input while hovering popup
            }

            // Click outside — close
            if (lclick && !hoverHeader) {
                isOpen = false;
                hoverItem = -1;
            }
        }

        return hoverHeader && (ldown || lclick);
    }

    void Render(LightningEngine::Renderer& r, TitanFont& f, float ox, float oy) override
    {
        if (!visible) return;
        float ax = ox+x, ay = oy+y;
        const Style& s = gStyle;

        // --- Header ---
        Color bg = (hoverHeader || isOpen) ? s.btnHover : s.btnNormal;
        r.SetDrawColor(bg.r, bg.g, bg.b);
        r.FillRect(ax, ay, w, h);

        r.SetDrawColor(s.btnBorder.r, s.btnBorder.g, s.btnBorder.b);
        r.DrawRect(ax, ay, w, h);

        // Header text
        const std::string& txt = (selectedIndex >= 0 && selectedIndex < (int)items.size())
                                 ? items[selectedIndex] : placeholder;
        float ty = ay + (h - f.GlyphH()) * 0.5f;
        r.SetDrawColor(s.btnText.r, s.btnText.g, s.btnText.b);
        f.DrawText(r, txt.c_str(), ax + s.padding, ty);

        // Arrow "v"
        float arrowX = ax + w - kArrowW * 0.5f - s.padding;
        r.SetDrawColor(s.textDim.r, s.textDim.g, s.textDim.b);
        f.DrawText(r, isOpen ? "^" : "v", arrowX, ty);

        // --- Popup list ---
        if (isOpen && !items.empty()) {
            float popY = ay + h;
            float popH = kItemH * (float)items.size();

            // Background
            r.SetDrawColor(s.menuBg.r, s.menuBg.g, s.menuBg.b);
            r.FillRect(ax, popY, w, popH);

            // Border
            r.SetDrawColor(s.menuBorder.r, s.menuBorder.g, s.menuBorder.b);
            r.DrawRect(ax, popY, w, popH);

            for (int i = 0; i < (int)items.size(); i++) {
                float iy = popY + i * kItemH;

                // Hover / selected highlight
                if (i == hoverItem) {
                    r.SetDrawColor(s.selection.r, s.selection.g, s.selection.b, s.selection.a);
                    r.FillRect(ax+1, iy, w-2, kItemH);
                } else if (i == selectedIndex) {
                    r.SetDrawColor(s.highlight.r, s.highlight.g, s.highlight.b, 80);
                    r.FillRect(ax+1, iy, w-2, kItemH);
                }

                // Item text
                Color tc = (i == hoverItem) ? s.textBright : s.textNormal;
                r.SetDrawColor(tc.r, tc.g, tc.b);
                f.DrawText(r, items[i].c_str(),
                           ax + s.padding,
                           iy + (kItemH - f.GlyphH()) * 0.5f);
            }
        }
    }
};

} // namespace Titan
