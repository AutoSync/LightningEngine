// MenuBar.h — Horizontal menu bar with dropdown menus.
#pragma once
#include <vector>
#include <string>
#include <functional>
#include "../Widget.h"
#include "../TitanStyle.h"
#include "../TitanFont.h"

namespace Titan {

struct MenuItem {
    std::string           label;
    std::function<void()> action;
    bool                  separator = false;
    bool                  enabled   = true;
};

struct Menu {
    std::string           label;
    std::vector<MenuItem> items;
    float                 headerX = 0.f;
    float                 headerW = 0.f;
};

class MenuBar : public Widget {
public:
    std::vector<Menu> menus;
    int  openIdx     = -1;
    int  hoverHeader = -1;
    int  hoverItem   = -1;

    static constexpr float kItemH  = 20.f;
    static constexpr float kSepH   =  5.f;
    static constexpr float kPadX   = 10.f;
    static constexpr float kDropW  = 160.f;

    MenuBar() = default;
    MenuBar(float x, float y, float w, float h)
    { this->x=x; this->y=y; this->w=w; this->h=h; }

    void AddMenu(const std::string& label)
    {
        Menu m;
        m.label = label;
        menus.push_back(std::move(m));
    }

    void AddItem(const std::string& menuLabel, const std::string& itemLabel,
                 std::function<void()> action, bool enabled = true)
    {
        for (auto& m : menus) {
            if (m.label == menuLabel) {
                MenuItem item;
                item.label   = itemLabel;
                item.action  = action;
                item.enabled = enabled;
                m.items.push_back(std::move(item));
                return;
            }
        }
    }

    void AddSeparator(const std::string& menuLabel)
    {
        for (auto& m : menus) {
            if (m.label == menuLabel) {
                MenuItem sep;
                sep.separator = true;
                m.items.push_back(std::move(sep));
                return;
            }
        }
    }

    bool ProcessInput(float mx, float my, bool ldown, bool lclick, bool lrelease,
                      TitanUI* ui, float ox, float oy) override
    {
        if (!visible || !enabled) return false;
        float ax = ox+x, ay = oy+y;

        // Compute header positions (reuse Render layout logic)
        computeHeaderPositions(ax);

        // Header hover
        hoverHeader = -1;
        for (int i = 0; i < (int)menus.size(); i++) {
            float hx = menus[i].headerX;
            float hw = menus[i].headerW;
            if (mx >= hx && mx < hx + hw && my >= ay && my < ay + h) {
                hoverHeader = i;
                break;
            }
        }

        // Header click: toggle open/close
        if (lclick && hoverHeader >= 0) {
            openIdx   = (openIdx == hoverHeader) ? -1 : hoverHeader;
            hoverItem = -1;
            if (openIdx >= 0) DoBringToFront(ui);
            return true;
        }

        // Dropdown interaction
        if (openIdx >= 0) {
            const Menu& m  = menus[openIdx];
            float dropX    = m.headerX;
            float dropY    = ay + h;
            float dropW    = kDropW;
            float dropH    = dropdownHeight(m);

            bool inDrop = mx >= dropX && mx < dropX + dropW &&
                          my >= dropY && my < dropY + dropH;

            if (inDrop) {
                // Determine which item is hovered
                hoverItem = -1;
                float iy  = dropY;
                for (int i = 0; i < (int)m.items.size(); i++) {
                    float ih = m.items[i].separator ? kSepH : kItemH;
                    if (!m.items[i].separator && my >= iy && my < iy + ih) {
                        hoverItem = i;
                        break;
                    }
                    iy += ih;
                }

                if (lclick && hoverItem >= 0) {
                    const MenuItem& item = m.items[hoverItem];
                    if (!item.separator && item.enabled && item.action) {
                        item.action();
                    }
                    openIdx   = -1;
                    hoverItem = -1;
                    return true;
                }
                return true; // consume hover
            }

            // Click outside dropdown → close
            if (lclick) {
                openIdx   = -1;
                hoverItem = -1;
            }
            return openIdx >= 0; // block input while open
        }

        // Consume clicks on the bar itself
        bool inBar = mx >= ax && mx < ax + w && my >= ay && my < ay + h;
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

        // Compute header widths from font
        float curX = ax;
        for (int i = 0; i < (int)menus.size(); i++) {
            float lw = f.MeasureW(menus[i].label.c_str());
            menus[i].headerW = lw + kPadX * 2.f;
            menus[i].headerX = curX;
            curX += menus[i].headerW;
        }

        // Draw headers
        for (int i = 0; i < (int)menus.size(); i++) {
            float hx = menus[i].headerX;
            float hw = menus[i].headerW;
            bool  active = (i == openIdx || i == hoverHeader);

            Color bg = active ? s.highlight : s.btnNormal;
            r.SetDrawColor(bg.r, bg.g, bg.b);
            r.FillRect(hx, ay, hw, h);

            Color tc = (i == openIdx) ? s.textBright : s.textNormal;
            r.SetDrawColor(tc.r, tc.g, tc.b);
            float lw = f.MeasureW(menus[i].label.c_str());
            f.DrawText(r, menus[i].label.c_str(),
                       hx + kPadX,
                       ay + (h - f.GlyphH()) * 0.5f);
        }

        // Bar border (bottom line)
        r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b);
        r.DrawRect(ax, ay, w, h);

        // Dropdown
        if (openIdx >= 0 && openIdx < (int)menus.size()) {
            const Menu& m = menus[openIdx];
            float dropX   = m.headerX;
            float dropY   = ay + h;
            float dropW   = kDropW;
            float dropH   = dropdownHeight(m);

            // Background
            r.SetDrawColor(s.menuBg.r, s.menuBg.g, s.menuBg.b);
            r.FillRect(dropX, dropY, dropW, dropH);

            // Items
            float iy = dropY;
            for (int i = 0; i < (int)m.items.size(); i++) {
                const MenuItem& item = m.items[i];
                if (item.separator) {
                    r.SetDrawColor(s.separator.r, s.separator.g, s.separator.b);
                    r.FillRect(dropX + 4.f, iy + kSepH * 0.5f - 1.f, dropW - 8.f, 1.f);
                    iy += kSepH;
                    continue;
                }

                if (i == hoverItem && item.enabled) {
                    r.SetDrawColor(s.selection.r, s.selection.g, s.selection.b, s.selection.a);
                    r.FillRect(dropX + 1.f, iy, dropW - 2.f, kItemH);
                }

                Color tc;
                if (!item.enabled)    tc = s.textDim;
                else if (i==hoverItem) tc = s.textBright;
                else                   tc = s.textNormal;

                r.SetDrawColor(tc.r, tc.g, tc.b);
                f.DrawText(r, item.label.c_str(),
                           dropX + kPadX,
                           iy + (kItemH - f.GlyphH()) * 0.5f);
                iy += kItemH;
            }

            // Border
            r.SetDrawColor(s.menuBorder.r, s.menuBorder.g, s.menuBorder.b);
            r.DrawRect(dropX, dropY, dropW, dropH);
        }
    }

private:
    // Recompute header positions without font (approximate, used in ProcessInput)
    void computeHeaderPositions(float startX)
    {
        // Uses stored headerW if already computed by Render; otherwise uses a fallback.
        float curX = startX;
        for (auto& m : menus) {
            if (m.headerW <= 0.f) m.headerW = (float)m.label.size() * 8.f + kPadX * 2.f;
            m.headerX = curX;
            curX += m.headerW;
        }
    }

    float dropdownHeight(const Menu& m) const
    {
        float h = 0.f;
        for (auto& item : m.items)
            h += item.separator ? kSepH : kItemH;
        return h;
    }
};

} // namespace Titan
