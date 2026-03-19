// Window.h — Floating, draggable, optionally-resizable panel.
// Title bar: drag handle. Set resizable=true for 8-direction resize handles.
#pragma once
#include <string>
#include <functional>
#include <algorithm>
#include "../Widget.h"
#include "../TitanStyle.h"
#include "../TitanFont.h"

namespace Titan {

class Window : public Widget {
public:
    std::string title;
    bool  closable  = true;
    bool  resizable = false;

    float minW = 120.f;
    float minH =  60.f;

    std::function<void()> onClose;

    static constexpr float kCloseW  = 16.f;
    static constexpr float kHandleZ =  6.f;  // resize handle depth in px

    Window() = default;
    Window(float x, float y, float w, float h, const char* title = "Window")
        : title(title ? title : "Window")
    { this->x=x; this->y=y; this->w=w; this->h=h; }

    bool ProcessInput(float mx, float my, bool ldown, bool lclick, bool lrelease,
                      TitanUI* ui, float ox, float oy) override
    {
        if (!visible || !enabled) return false;
        float ax = ox+x, ay = oy+y;
        float titleH = gStyle.titleH;

        // ── Close button ──────────────────────────────────────────────────
        if (closable) {
            float cx = ax + w - kCloseW - gStyle.padding;
            float cy = ay + (titleH - kCloseW) * 0.5f;
            hoverClose = mx >= cx && mx < cx+kCloseW && my >= cy && my < cy+kCloseW;
            if (hoverClose && lclick) {
                if (onClose) onClose();
                visible = false;
                return true;
            }
        }

        // ── Active resize drag ────────────────────────────────────────────
        if (resizing) {
            if (ldown) {
                applyResize(mx, my, ox, oy);
            } else {
                resizing = false;
                SetUICapture(ui, nullptr);
            }
            return true;
        }

        // ── Start resize (click on edge/corner) ───────────────────────────
        if (resizable && lclick && !hoverClose) {
            Edge e = hitEdge(mx, my, ax, ay);
            if (e != Edge::None) {
                resizeEdge = e;
                resizing   = true;
                resizeStartX = mx; resizeStartY = my;
                resizeOrigX  = x;  resizeOrigY  = y;
                resizeOrigW  = w;  resizeOrigH  = h;
                SetUICapture(ui, this);
                return true;
            }
        }

        // ── Title bar drag ────────────────────────────────────────────────
        bool inTitle = mx >= ax && mx < ax+w && my >= ay && my < ay+titleH;
        if (inTitle && lclick && !hoverClose) {
            dragging = true;
            dragOffX = mx - x;
            dragOffY = my - y;
            SetUICapture(ui, this);
        }
        if (dragging) {
            x = std::max(0.f, mx - dragOffX);
            y = std::max(0.f, my - dragOffY);
            if (!ldown) { dragging = false; SetUICapture(ui, nullptr); }
            return true;
        }

        // ── Forward to children ───────────────────────────────────────────
        if (Contains(mx, my, ox, oy)) {
            for (int i = (int)children.size()-1; i >= 0; i--)
                if (children[i]->ProcessInput(mx, my, ldown, lclick, lrelease, ui, ax, ay))
                    return true;
            return ldown || lclick;
        }
        return false;
    }

    void Render(LightningEngine::Renderer& r, TitanFont& f, float ox, float oy) override
    {
        if (!visible) return;
        float ax = ox+x, ay = oy+y;
        const Style& s   = gStyle;
        float titleH = s.titleH;

        // Shadow
        r.SetDrawColor(8, 8, 12, 160);
        r.FillRect(ax+4.f, ay+4.f, w, h);

        // Body
        r.SetDrawColor(s.panelBg.r, s.panelBg.g, s.panelBg.b);
        r.FillRect(ax, ay, w, h);

        // Title bar
        r.SetDrawColor(s.panelHeader.r, s.panelHeader.g, s.panelHeader.b);
        r.FillRect(ax, ay, w, titleH);

        // Title text
        r.SetDrawColor(s.textBright.r, s.textBright.g, s.textBright.b);
        f.DrawText(r, title.c_str(), ax + s.padding, ay + (titleH - f.GlyphH()) * 0.5f);

        // Close button
        if (closable) {
            float cx = ax + w - kCloseW - s.padding;
            float cy = ay + (titleH - kCloseW) * 0.5f;
            if (hoverClose) {
                r.SetDrawColor(185, 48, 48);
                r.FillRect(cx, cy, kCloseW, kCloseW);
            }
            r.SetDrawColor(hoverClose ? s.textBright.r : s.textDim.r,
                           hoverClose ? s.textBright.g : s.textDim.g,
                           hoverClose ? s.textBright.b : s.textDim.b);
            f.DrawText(r, "x", cx + (kCloseW - f.MeasureW("x")) * 0.5f,
                                cy + (kCloseW - f.GlyphH()) * 0.5f);
        }

        // Resize grip indicator (bottom-right corner, when resizable)
        if (resizable) {
            float gx = ax + w - 10.f;
            float gy = ay + h - 10.f;
            r.SetDrawColor(s.textDim.r, s.textDim.g, s.textDim.b, 120);
            for (int i = 0; i < 3; i++) {
                r.FillRect(gx + i*3.f, gy + 8.f, 2.f, 2.f);
                r.FillRect(gx + 8.f,   gy + i*3.f, 2.f, 2.f);
            }
        }

        // Border
        r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b);
        r.DrawRect(ax, ay, w, h);
        // Title separator
        r.SetDrawColor(s.separator.r, s.separator.g, s.separator.b);
        r.FillRect(ax, ay + titleH - 1.f, w, 1.f);

        // Children
        for (auto& c : children) c->Render(r, f, ax, ay);
    }

private:
    enum class Edge { None, N, S, E, W, NE, NW, SE, SW };

    bool  hoverClose    = false;
    bool  dragging      = false;
    float dragOffX      = 0.f, dragOffY = 0.f;
    bool  resizing      = false;
    Edge  resizeEdge    = Edge::None;
    float resizeStartX  = 0.f, resizeStartY = 0.f;
    float resizeOrigX   = 0.f, resizeOrigY  = 0.f;
    float resizeOrigW   = 0.f, resizeOrigH  = 0.f;

    Edge hitEdge(float mx, float my, float ax, float ay) const
    {
        float z = kHandleZ;
        bool onL = mx >= ax      && mx < ax+z;
        bool onR = mx >= ax+w-z  && mx < ax+w;
        bool onT = my >= ay      && my < ay+z;
        bool onB = my >= ay+h-z  && my < ay+h;
        bool inX = mx >= ax && mx < ax+w;
        bool inY = my >= ay && my < ay+h;

        if (onT && onL) return Edge::NW;
        if (onT && onR) return Edge::NE;
        if (onB && onL) return Edge::SW;
        if (onB && onR) return Edge::SE;
        if (onT && inX) return Edge::N;
        if (onB && inX) return Edge::S;
        if (onL && inY) return Edge::W;
        if (onR && inY) return Edge::E;
        return Edge::None;
    }

    void applyResize(float mx, float my, float ox, float oy)
    {
        float dx = mx - resizeStartX;
        float dy = my - resizeStartY;

        float nx = resizeOrigX, ny = resizeOrigY;
        float nw = resizeOrigW, nh = resizeOrigH;

        switch (resizeEdge) {
        case Edge::E:  nw = std::max(minW, resizeOrigW + dx); break;
        case Edge::S:  nh = std::max(minH, resizeOrigH + dy); break;
        case Edge::W:  nw = std::max(minW, resizeOrigW - dx); nx = resizeOrigX + (resizeOrigW - nw); break;
        case Edge::N:  nh = std::max(minH, resizeOrigH - dy); ny = resizeOrigY + (resizeOrigH - nh); break;
        case Edge::SE: nw = std::max(minW, resizeOrigW + dx); nh = std::max(minH, resizeOrigH + dy); break;
        case Edge::SW: nw = std::max(minW, resizeOrigW - dx); nx = resizeOrigX + (resizeOrigW - nw);
                       nh = std::max(minH, resizeOrigH + dy); break;
        case Edge::NE: nw = std::max(minW, resizeOrigW + dx);
                       nh = std::max(minH, resizeOrigH - dy); ny = resizeOrigY + (resizeOrigH - nh); break;
        case Edge::NW: nw = std::max(minW, resizeOrigW - dx); nx = resizeOrigX + (resizeOrigW - nw);
                       nh = std::max(minH, resizeOrigH - dy); ny = resizeOrigY + (resizeOrigH - nh); break;
        default: break;
        }
        x = std::max(0.f, nx - ox);
        y = std::max(0.f, ny - oy);
        w = nw;
        h = nh;
    }
};

} // namespace Titan
