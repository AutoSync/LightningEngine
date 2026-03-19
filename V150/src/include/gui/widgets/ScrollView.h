// ScrollView.h — Scrollable container with a vertical scrollbar.
// Children are rendered clipped to the visible viewport via soft-clipping
// (children whose bottom edge is above the viewport top, or top edge is
// below the viewport bottom, are simply not rendered).
// Supports mouse-wheel scrolling and scrollbar thumb dragging.
#pragma once
#include <algorithm>
#include "../Widget.h"
#include "../TitanStyle.h"
#include "../TitanFont.h"

namespace Titan {

class ScrollView : public Widget {
public:
    float scrollOffset  = 0.f;   // pixels scrolled from top
    float contentHeight = 0.f;   // total height of all children (set manually or auto-computed)
    bool  autoContent   = true;  // if true, contentHeight is derived from children bounds each frame

    static constexpr float kScrollbarW   = 8.f;
    static constexpr float kWheelSpeed   = 20.f;

    // Scrollbar drag state
    bool  sbDragging    = false;
    float sbDragStartY  = 0.f;
    float sbDragStartOff= 0.f;

    ScrollView() = default;
    ScrollView(float x, float y, float w, float h)
    { this->x=x; this->y=y; this->w=w; this->h=h; }

    // Ensure scrollOffset stays within valid range
    void ClampScroll()
    {
        float maxOff = contentHeight > h ? contentHeight - h : 0.f;
        if (scrollOffset < 0.f)    scrollOffset = 0.f;
        if (scrollOffset > maxOff) scrollOffset = maxOff;
    }

    bool ProcessInput(float mx, float my, bool ldown, bool lclick, bool lrelease,
                      TitanUI* ui, float ox, float oy) override
    {
        if (!visible || !enabled) return false;
        float ax = ox+x, ay = oy+y;

        bool inView = mx >= ax && mx < ax+w && my >= ay && my < ay+h;

        // --- Mouse wheel ---
        if (inView && ui && GetUIScrollY(ui) != 0.f) {
            scrollOffset -= GetUIScrollY(ui) * kWheelSpeed;
            ClampScroll();
        }

        // --- Scrollbar dragging ---
        float sbX = ax + w - kScrollbarW;
        float visH = h;
        float maxOff = contentHeight > visH ? contentHeight - visH : 0.f;

        if (contentHeight > visH) {
            float thumbH  = visH * (visH / contentHeight);
            if (thumbH < 12.f) thumbH = 12.f;
            float thumbY  = ay + (scrollOffset / maxOff) * (visH - thumbH);

            bool inThumb = mx >= sbX && mx < sbX + kScrollbarW
                        && my >= thumbY && my < thumbY + thumbH;

            if (lclick && inThumb) {
                sbDragging     = true;
                sbDragStartY   = my;
                sbDragStartOff = scrollOffset;
                SetUICapture(ui, this);
            }

            if (sbDragging) {
                float delta = my - sbDragStartY;
                float scale = (visH - thumbH) > 0.f
                              ? maxOff / (visH - thumbH)
                              : 0.f;
                scrollOffset = sbDragStartOff + delta * scale;
                ClampScroll();
                if (!ldown) { sbDragging = false; SetUICapture(ui, nullptr); }
                return true;
            }
        }

        // --- Forward to children (with translated & soft-clipped input) ---
        float contentOX = ax;
        float contentOY = ay - scrollOffset;

        if (inView) {
            for (int i = (int)children.size()-1; i >= 0; i--) {
                Widget* c = children[i].get();
                // Only forward input if child is at least partly in view
                float childTop    = contentOY + c->y;
                float childBottom = childTop + c->h;
                if (childBottom < ay || childTop > ay + visH) continue;

                if (c->ProcessInput(mx, my, ldown, lclick, lrelease, ui,
                                    contentOX, contentOY))
                    return true;
            }
        }

        return inView && (ldown || lclick);
    }

    void Render(LightningEngine::Renderer& r, TitanFont& f, float ox, float oy) override
    {
        if (!visible) return;
        float ax = ox+x, ay = oy+y;
        const Style& s = gStyle;

        // Auto-compute content height from children
        if (autoContent && !children.empty()) {
            float maxBottom = 0.f;
            for (auto& c : children) {
                float b = c->y + c->h;
                if (b > maxBottom) maxBottom = b;
            }
            contentHeight = maxBottom;
        }

        ClampScroll();

        float visH = h;

        // Background
        r.SetDrawColor(s.panelBg.r, s.panelBg.g, s.panelBg.b);
        r.FillRect(ax, ay, w, visH);

        // Render children with offset (soft-clip: skip those fully outside)
        float contentOX = ax;
        float contentOY = ay - scrollOffset;

        for (auto& c : children) {
            if (!c->visible) continue;
            float childTop    = contentOY + c->y;
            float childBottom = childTop + c->h;
            if (childBottom < ay || childTop > ay + visH) continue;
            c->Render(r, f, contentOX, contentOY);
        }

        // Border
        r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b);
        r.DrawRect(ax, ay, w, visH);

        // Scrollbar
        if (contentHeight > visH) {
            float maxOff = contentHeight - visH;
            float thumbH = visH * (visH / contentHeight);
            if (thumbH < 12.f) thumbH = 12.f;
            float thumbY = ay + (maxOff > 0.f ? (scrollOffset / maxOff) : 0.f)
                               * (visH - thumbH);
            float sbX = ax + w - kScrollbarW;

            // Track
            r.SetDrawColor(s.sliderTrack.r, s.sliderTrack.g, s.sliderTrack.b);
            r.FillRect(sbX, ay, kScrollbarW, visH);

            // Thumb
            Color tc = sbDragging ? s.sliderFill : s.sliderThumb;
            r.SetDrawColor(tc.r, tc.g, tc.b);
            r.FillRect(sbX + 1.f, thumbY, kScrollbarW - 2.f, thumbH);
        }
    }
};

} // namespace Titan
