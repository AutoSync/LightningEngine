// ContextMenu.h — Popup context menu triggered at an arbitrary screen position.
// Add items with AddItem(). Call Open(x, y) to show at position.
// Closes automatically when an item is clicked or when clicking outside.
//
// Usage:
//   auto* ctx = ui.AddRoot<ContextMenu>();  // add once as root
//   ctx->AddItem("Cut",   [](){ ... });
//   ctx->AddItem("Copy",  [](){ ... });
//   ctx->AddItem("Paste", [](){ ... });
//   ctx->AddSeparator();
//   ctx->AddItem("Delete", [](){ ... });
//
//   // On right-click:
//   if (im.IsMousePressed(3)) ctx->Open(im.GetMouseX(), im.GetMouseY());
#pragma once
#include <vector>
#include <string>
#include <functional>
#include "../Widget.h"
#include "../TitanStyle.h"
#include "../TitanFont.h"

namespace Titan {

class ContextMenu : public Widget {
public:
    struct Item {
        std::string text;
        bool        separator = false;
        bool        enabled   = true;
        std::function<void()> action;
    };

    std::vector<Item> items;
    int   hoverIndex = -1;

    static constexpr float kItemH = 20.f;
    static constexpr float kSepH  = 5.f;
    static constexpr float kMinW  = 120.f;
    static constexpr float kPadX  = 10.f;

    ContextMenu() { visible = false; zOrder = 9000; }

    void AddItem(const char* text, std::function<void()> action = nullptr,
                 bool enabled = true)
    {
        items.push_back({ text ? text : "", false, enabled, std::move(action) });
    }

    void AddSeparator()
    {
        items.push_back({ "", true, false, nullptr });
    }

    void Open(float px, float py)
    {
        x = px; y = py;
        visible = true;
        hoverIndex = -1;
        rebuildSize();
    }

    void Close() { visible = false; hoverIndex = -1; }

    bool ProcessInput(float mx, float my, bool ldown, bool lclick, bool lrelease,
                      TitanUI* ui, float ox, float oy) override
    {
        if (!visible) return false;
        float ax = ox+x, ay = oy+y;
        bool inMenu = mx >= ax && mx < ax+w && my >= ay && my < ay+h;

        if (!inMenu) {
            // Click outside closes the menu
            if (lclick) Close();
            return false;
        }

        // Find hovered item
        hoverIndex = -1;
        float iy = ay;
        for (int i = 0; i < (int)items.size(); i++) {
            float ih = items[i].separator ? kSepH : kItemH;
            if (!items[i].separator && my >= iy && my < iy + ih) {
                if (items[i].enabled) hoverIndex = i;
            }
            iy += ih;
        }

        // Click fires action
        if (lclick && hoverIndex >= 0) {
            if (items[hoverIndex].action) items[hoverIndex].action();
            Close();
            return true;
        }

        return true; // consume all input while open
    }

    void Render(LightningEngine::Renderer& r, TitanFont& f, float ox, float oy) override
    {
        if (!visible) return;
        // Rebuild size using font (first render after Open)
        rebuildSizeWithFont(f);

        float ax = ox+x, ay = oy+y;
        const Style& s = gStyle;

        // Shadow
        r.SetDrawColor(0, 0, 0, 100);
        r.FillRect(ax + 3.f, ay + 3.f, w, h);

        // Background
        r.SetDrawColor(s.menuBg.r, s.menuBg.g, s.menuBg.b);
        r.FillRect(ax, ay, w, h);

        // Items
        float iy = ay;
        for (int i = 0; i < (int)items.size(); i++) {
            const Item& item = items[i];

            if (item.separator) {
                float sy = iy + kSepH * 0.5f;
                r.SetDrawColor(s.separator.r, s.separator.g, s.separator.b);
                r.FillRect(ax + kPadX, sy, w - kPadX * 2.f, 1.f);
                iy += kSepH;
                continue;
            }

            // Hover highlight
            if (i == hoverIndex) {
                r.SetDrawColor(s.highlight.r, s.highlight.g, s.highlight.b);
                r.FillRect(ax + 2.f, iy, w - 4.f, kItemH);
            }

            // Text
            Color tc = !item.enabled ? s.textDim
                     : (i == hoverIndex ? s.textBright : s.textNormal);
            r.SetDrawColor(tc.r, tc.g, tc.b);
            f.DrawText(r, item.text.c_str(),
                       ax + kPadX,
                       iy + (kItemH - f.GlyphH()) * 0.5f);

            iy += kItemH;
        }

        // Border
        r.SetDrawColor(s.menuBorder.r, s.menuBorder.g, s.menuBorder.b);
        r.DrawRect(ax, ay, w, h);
    }

private:
    void rebuildSize()
    {
        float totalH = 0.f;
        for (auto& it : items)
            totalH += it.separator ? kSepH : kItemH;
        h = totalH;
        w = kMinW;
    }

    void rebuildSizeWithFont(TitanFont& f)
    {
        float maxW = kMinW;
        float totalH = 0.f;
        for (auto& it : items) {
            if (!it.separator) {
                float tw = f.MeasureW(it.text.c_str()) + kPadX * 2.f;
                if (tw > maxW) maxW = tw;
            }
            totalH += it.separator ? kSepH : kItemH;
        }
        w = maxW;
        h = totalH;
    }
};

} // namespace Titan
