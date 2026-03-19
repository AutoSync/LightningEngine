// TabStrip.h — Horizontal tab strip + content area.
// Each tab has a label, optional color accent (underline), and optional close button.
// Nav arrows (< >) appear automatically when tabs overflow the available width.
//
// Usage:
//   auto* tabs = panel->Add<TabStrip>(x, y, w, h);
//   auto* t0 = tabs->AddTab("Scene");
//   t0->Add<Label>(...);   // children of tab 0
//   auto* t1 = tabs->AddTab("Assets", 255, 138, 28, /*closable=*/true);
//   tabs->onClose = [](int i) { /* remove tab i */ };
#pragma once
#include <vector>
#include <string>
#include <functional>
#include "../Widget.h"
#include "../TitanStyle.h"
#include "../TitanFont.h"

namespace Titan {

// Represents one tab's content container.
struct Tab {
    std::string label;
    Uint8 accentR = 255, accentG = 255, accentB = 255;
    bool  closable = false;
    Widget* content = nullptr;  // non-owning pointer into TabStrip::contents
};

class TabStrip : public Widget {
public:
    std::vector<Tab>                       tabs;
    std::vector<std::unique_ptr<Widget>>   contents; // owns content widgets
    int   activeTab   = 0;
    int   hoverTab    = -1;
    int   hoverClose  = -1;  // index of tab whose close btn is hovered
    int   hoverNav    = 0;   // -1 = left arrow hovered, 1 = right arrow hovered
    int   scrollFirst = 0;   // first visible tab index (for overflow scrolling)
    float tabH        = 0.f; // 0 = gStyle.titleH
    float tabPadX     = 10.f;
    bool  equalW      = false;
    std::function<void(int)> onTabChanged;
    std::function<void(int)> onClose;  // called with tab index when × clicked

    // Cached per-tab widths (updated each Render when font is available)
    mutable std::vector<float> tabWidths;

    static constexpr float kCloseW  = 14.f;  // close button area width per tab
    static constexpr float kNavColW = 18.f;  // width of the vertical nav column
    // Nav arrows are stacked vertically in a single kNavColW column (per tabs_mock spec)

    TabStrip() = default;
    TabStrip(float x, float y, float w, float h, float tabH = 0.f)
        : tabH(tabH)
    { this->x=x; this->y=y; this->w=w; this->h=h; }

    // Add a new tab and return its content widget container.
    Widget* AddTab(const char* label,
                   Uint8 accentR=255, Uint8 accentG=255, Uint8 accentB=255,
                   bool closable=false)
    {
        Tab t;
        t.label    = label ? label : "";
        t.accentR  = accentR;
        t.accentG  = accentG;
        t.accentB  = accentB;
        t.closable = closable;

        auto cw   = std::make_unique<Widget>();
        t.content = cw.get();
        contents.push_back(std::move(cw));
        tabs.push_back(t);
        return t.content;
    }

    void SetActive(int idx)
    {
        if (idx >= 0 && idx < (int)tabs.size()) {
            activeTab = idx;
            if (onTabChanged) onTabChanged(activeTab);
        }
    }

    // Remove tab at index, adjusting activeTab and scrollFirst accordingly.
    void RemoveTab(int idx)
    {
        if (idx < 0 || idx >= (int)tabs.size()) return;
        tabs.erase(tabs.begin() + idx);
        contents.erase(contents.begin() + idx);
        tabWidths.clear();
        if (activeTab >= (int)tabs.size()) activeTab = (int)tabs.size() - 1;
        if (activeTab < 0) activeTab = 0;
        if (scrollFirst >= (int)tabs.size()) scrollFirst = (int)tabs.size() - 1;
        if (scrollFirst < 0) scrollFirst = 0;
    }

    bool ProcessInput(float mx, float my, bool ldown, bool lclick, bool lrelease,
                      TitanUI* ui, float ox, float oy) override
    {
        if (!visible || !enabled) return false;
        float ax = ox+x, ay = oy+y;
        float th       = tabH > 0.f ? tabH : gStyle.titleH;
        float contentY = ay + th;
        float contentH = h - th;

        bool  overflow = hasOverflow();
        float usableW  = overflow ? w - kNavColW : w;

        // Nav column: single kNavColW column on the right, split in half vertically
        // Top half = scroll right (>), Bottom half = scroll left (<)  — per tabs_mock
        hoverNav = 0;
        if (overflow && my >= ay && my < ay + th) {
            float nx = ax + usableW;
            float midY = ay + th * 0.5f;
            if (mx >= nx && mx < nx + kNavColW) {
                if (my < midY) {
                    hoverNav = 1;   // top = scroll right
                    if (lclick) {
                        int maxFirst = (int)tabs.size() - countVisibleTabs(usableW);
                        if (maxFirst < 0) maxFirst = 0;
                        if (scrollFirst < maxFirst) scrollFirst++;
                        return true;
                    }
                } else {
                    hoverNav = -1;  // bottom = scroll left
                    if (lclick && scrollFirst > 0) { scrollFirst--; return true; }
                }
                return true;
            }
        }

        // Tab bar hit testing
        hoverTab   = -1;
        hoverClose = -1;
        float tx = ax;
        for (int i = scrollFirst; i < (int)tabs.size(); i++) {
            float tw = (i < (int)tabWidths.size()) ? tabWidths[i] : 80.f;
            if (tx + tw > ax + usableW + 0.5f) break;

            bool inTab = mx >= tx && mx < tx + tw && my >= ay && my < ay + th;
            if (inTab) {
                hoverTab = i;
                // Close button hit test (right edge of closable tab)
                if (tabs[i].closable) {
                    float cbx = tx + tw - kCloseW;
                    if (mx >= cbx && mx < cbx + kCloseW) {
                        hoverClose = i;
                        if (lclick) {
                            if (onClose) onClose(i);
                            return true;
                        }
                        return true;
                    }
                }
                if (lclick) SetActive(i);
                return true;
            }
            tx += tw;
        }

        // Forward input to active tab's content area
        if (activeTab >= 0 && activeTab < (int)tabs.size()) {
            Widget* c = tabs[activeTab].content;
            if (c) return c->ProcessInput(mx, my, ldown, lclick, lrelease,
                                          ui, ax, contentY);
        }

        // Consume clicks inside content area
        bool inContent = mx >= ax && mx < ax+w
                      && my >= contentY && my < contentY + contentH;
        return inContent && (ldown || lclick);
    }

    void Render(LightningEngine::Renderer& r, TitanFont& f, float ox, float oy) override
    {
        if (!visible) return;
        float ax = ox+x, ay = oy+y;
        const Style& s = gStyle;
        float th = tabH > 0.f ? tabH : s.titleH;

        rebuildWidths(f);

        bool  overflow = hasOverflow();
        float usableW  = overflow ? w - kNavColW : w;

        // Tab bar background
        r.SetDrawColor(s.panelHeader.r, s.panelHeader.g, s.panelHeader.b);
        r.FillRect(ax, ay, usableW, th);

        // Draw tabs
        float tx = ax;
        for (int i = scrollFirst; i < (int)tabs.size(); i++) {
            float tw = tabWidths[i];
            if (tx + tw > ax + usableW + 0.5f) break;

            bool sel      = (i == activeTab);
            bool hov      = (i == hoverTab && hoverClose < 0);
            const Tab& tab = tabs[i];

            // Tab background
            if (sel) {
                r.SetDrawColor(s.panelBg.r, s.panelBg.g, s.panelBg.b);
                r.FillRect(tx, ay, tw, th);
            } else if (hov) {
                r.SetDrawColor(s.btnHover.r, s.btnHover.g, s.btnHover.b);
                r.FillRect(tx, ay, tw, th);
            }

            // Tab label
            Color lc = sel ? Color{tab.accentR, tab.accentG, tab.accentB} : s.textDim;
            r.SetDrawColor(lc.r, lc.g, lc.b);
            f.DrawText(r, tab.label.c_str(),
                       tx + tabPadX,
                       ay + (th - f.GlyphH()) * 0.5f);

            // Active tab accent underline
            if (sel) {
                r.SetDrawColor(tab.accentR, tab.accentG, tab.accentB);
                r.FillRect(tx, ay + th - 2.f, tw, 2.f);
            }

            // Close button ○ for closable tabs — small hollow square (circle approximation)
            if (tab.closable) {
                float cbSz = 7.f;
                float cbx  = tx + tw - kCloseW + (kCloseW - cbSz) * 0.5f;
                float cby  = ay + (th - cbSz) * 0.5f;
                bool  chov = (i == hoverClose);
                if (chov) {
                    r.SetDrawColor(s.textRed.r, s.textRed.g, s.textRed.b);
                } else if (sel) {
                    r.SetDrawColor(s.textDim.r, s.textDim.g, s.textDim.b);
                } else {
                    r.SetDrawColor(s.separator.r, s.separator.g, s.separator.b);
                }
                r.DrawRect(cbx, cby, cbSz, cbSz);  // hollow square ≈ ○
            }

            // Right-edge separator between tabs
            r.SetDrawColor(s.separator.r, s.separator.g, s.separator.b);
            r.FillRect(tx + tw - 1.f, ay, 1.f, th);

            tx += tw;
        }

        // Tab bar bottom border (full width)
        r.SetDrawColor(s.separator.r, s.separator.g, s.separator.b);
        r.FillRect(ax, ay + th - 1.f, w, 1.f);

        // Nav column — single column (kNavColW wide), vertically split:
        //   Top half  = scroll right (>)
        //   Bottom half = scroll left  (<)
        // Per tabs_mock: arrows stacked in one narrow column before ProjectName
        if (overflow) {
            float nx    = ax + usableW;
            float halfH = th * 0.5f;
            float gh    = f.GlyphH();
            int maxFirst = (int)tabs.size() - countVisibleTabs(usableW);
            if (maxFirst < 0) maxFirst = 0;

            // Column background
            r.SetDrawColor(s.btnNormal.r, s.btnNormal.g, s.btnNormal.b);
            r.FillRect(nx, ay, kNavColW, th);

            // Left border separator
            r.SetDrawColor(s.separator.r, s.separator.g, s.separator.b);
            r.FillRect(nx, ay, 1.f, th);
            // Mid separator
            r.FillRect(nx + 1.f, ay + halfH - 0.5f, kNavColW - 1.f, 1.f);

            // Top half: scroll right (>)
            if (hoverNav == 1) {
                r.SetDrawColor(s.btnHover.r, s.btnHover.g, s.btnHover.b);
                r.FillRect(nx + 1.f, ay, kNavColW - 1.f, halfH);
            }
            Color rtc = (scrollFirst < maxFirst) ? s.textNormal : s.textDim;
            r.SetDrawColor(rtc.r, rtc.g, rtc.b);
            f.DrawText(r, ">", nx + (kNavColW - f.MeasureW(">")) * 0.5f,
                       ay + (halfH - gh) * 0.5f);

            // Bottom half: scroll left (<)
            if (hoverNav == -1) {
                r.SetDrawColor(s.btnHover.r, s.btnHover.g, s.btnHover.b);
                r.FillRect(nx + 1.f, ay + halfH, kNavColW - 1.f, halfH);
            }
            Color ltc = (scrollFirst > 0) ? s.textNormal : s.textDim;
            r.SetDrawColor(ltc.r, ltc.g, ltc.b);
            f.DrawText(r, "<", nx + (kNavColW - f.MeasureW("<")) * 0.5f,
                       ay + halfH + (halfH - gh) * 0.5f);
        }

        // Content area (only if there is vertical space below the tab bar)
        float contentY = ay + th;
        float contentH = h - th;
        if (contentH > 0.f) {
            r.SetDrawColor(s.panelBg.r, s.panelBg.g, s.panelBg.b);
            r.FillRect(ax, contentY, w, contentH);

            if (activeTab >= 0 && activeTab < (int)tabs.size()) {
                Widget* c = tabs[activeTab].content;
                if (c) c->Render(r, f, ax, contentY);
            }

            r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b);
            r.DrawRect(ax, ay, w, h);
        }
    }

    // Return the accumulated start-X of tab i (relative to widget origin, post-scroll).
    float tabStartX(int i) const
    {
        float tx = 0.f;
        for (int j = scrollFirst; j < i && j < (int)tabWidths.size(); j++)
            tx += tabWidths[j];
        return tx;
    }

private:
    void rebuildWidths(TitanFont& f) const
    {
        tabWidths.resize(tabs.size());
        if (equalW && !tabs.empty()) {
            float tw = w / (float)tabs.size();
            for (auto& v : tabWidths) v = tw;
        } else {
            for (int i = 0; i < (int)tabs.size(); i++) {
                float lw = f.MeasureW(tabs[i].label.c_str()) + tabPadX * 2.f;
                if (tabs[i].closable) lw += kCloseW;
                tabWidths[i] = lw;
            }
        }
    }

    bool hasOverflow() const
    {
        if (tabWidths.empty()) return false;
        float total = 0.f;
        for (float tw : tabWidths) total += tw;
        return total > w;
    }

    // How many tabs fit in availW starting from scrollFirst.
    int countVisibleTabs(float availW) const
    {
        float total = 0.f;
        int   count = 0;
        for (int i = scrollFirst; i < (int)tabWidths.size(); i++) {
            total += tabWidths[i];
            if (total > availW) break;
            count++;
        }
        return count > 0 ? count : 1;
    }
};

} // namespace Titan
