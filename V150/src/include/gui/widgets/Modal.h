// Modal.h — Centered confirmation dialog widget.
// Covers the full screen; starts hidden. Call Open() to show.
#pragma once
#include <string>
#include <functional>
#include "../Widget.h"
#include "../TitanStyle.h"
#include "../TitanFont.h"

namespace Titan {

class Modal : public Widget {
public:
    std::string title;
    std::string message;
    std::string confirmText = "OK";
    std::string cancelText  = "Cancel";
    bool        showCancel  = true;

    std::function<void()> onConfirm;
    std::function<void()> onCancel;

    Modal() { visible = false; }

    void Open(const std::string& t, const std::string& msg, bool cancel = true)
    {
        title       = t;
        message     = msg;
        showCancel  = cancel;
        visible     = true;
    }

    void Close() { visible = false; }

    bool ProcessInput(float mx, float my, bool ldown, bool lclick, bool lrelease,
                      TitanUI* ui, float ox, float oy) override
    {
        if (!visible) return false;

        // Store last mouse position for hover in Render
        lastMx = mx;
        lastMy = my;

        if (!lclick) return true; // block all input, no click yet

        // Compute dialog rect (same as Render)
        float dw   = 320.f;
        float dh   = showCancel ? 140.f : 110.f;
        float dx   = ox + x + (w - dw) * 0.5f;
        float dy   = oy + y + (h - dh) * 0.5f;

        float titleH = gStyle.titleH;
        float btnH   = gStyle.lineH + 6.f;
        float btnW   = 80.f;
        float pad    = gStyle.padding;

        // OK button rect
        float okX = dx + dw - btnW - pad;
        float okY = dy + dh - btnH - pad;

        // Cancel button rect
        float cancelX = okX - btnW - pad;
        float cancelY = okY;

        // Check OK click
        if (mx >= okX && mx < okX + btnW && my >= okY && my < okY + btnH) {
            Close();
            if (onConfirm) onConfirm();
            return true;
        }

        // Check Cancel click
        if (showCancel &&
            mx >= cancelX && mx < cancelX + btnW && my >= cancelY && my < cancelY + btnH)
        {
            Close();
            if (onCancel) onCancel();
            return true;
        }

        return true; // block everything while visible
    }

    void Render(LightningEngine::Renderer& r, TitanFont& f, float ox, float oy) override
    {
        if (!visible) return;
        float ax = ox+x, ay = oy+y;
        const Style& s = gStyle;

        // Dim backdrop
        r.SetDrawColor(0, 0, 0, 160);
        r.FillRect(ax, ay, w, h);

        // Dialog dimensions
        float dw = 320.f;
        float dh = showCancel ? 140.f : 110.f;
        float dx = ax + (w - dw) * 0.5f;
        float dy = ay + (h - dh) * 0.5f;

        float pad    = s.padding;
        float titleH = s.titleH;
        float btnH   = s.lineH + 6.f;
        float btnW   = 80.f;

        // Panel background
        r.SetDrawColor(s.panelBg.r, s.panelBg.g, s.panelBg.b);
        r.FillRect(dx, dy, dw, dh);

        // Title bar
        r.SetDrawColor(s.panelHeader.r, s.panelHeader.g, s.panelHeader.b);
        r.FillRect(dx, dy, dw, titleH);

        // Title text (centered)
        {
            float tw = f.MeasureW(title.c_str());
            float tx = dx + (dw - tw) * 0.5f;
            float ty = dy + (titleH - f.GlyphH()) * 0.5f;
            r.SetDrawColor(s.textBright.r, s.textBright.g, s.textBright.b);
            f.DrawText(r, title.c_str(), tx, ty);
        }

        // Message text (centered)
        {
            float tw = f.MeasureW(message.c_str());
            float tx = dx + (dw - tw) * 0.5f;
            float ty = dy + titleH + pad + (s.lineH - f.GlyphH()) * 0.5f;
            r.SetDrawColor(s.textNormal.r, s.textNormal.g, s.textNormal.b);
            f.DrawText(r, message.c_str(), tx, ty);
        }

        // Button positions
        float okX    = dx + dw - btnW - pad;
        float okY    = dy + dh - btnH - pad;
        float cancelX = okX - btnW - pad;
        float cancelY = okY;

        bool hoverOK = (lastMx >= okX && lastMx < okX + btnW &&
                        lastMy >= okY && lastMy < okY + btnH);
        bool hoverCancel = showCancel &&
                           (lastMx >= cancelX && lastMx < cancelX + btnW &&
                            lastMy >= cancelY && lastMy < cancelY + btnH);

        // OK button
        Color okBg = hoverOK ? s.highlight : s.btnActive;
        r.SetDrawColor(okBg.r, okBg.g, okBg.b);
        r.FillRect(okX, okY, btnW, btnH);
        r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b);
        r.DrawRect(okX, okY, btnW, btnH);
        {
            float tw = f.MeasureW(confirmText.c_str());
            r.SetDrawColor(s.textBright.r, s.textBright.g, s.textBright.b);
            f.DrawText(r, confirmText.c_str(),
                       okX + (btnW - tw) * 0.5f,
                       okY + (btnH - f.GlyphH()) * 0.5f);
        }

        // Cancel button
        if (showCancel) {
            Color cancelBg = hoverCancel ? s.btnHover : s.btnNormal;
            r.SetDrawColor(cancelBg.r, cancelBg.g, cancelBg.b);
            r.FillRect(cancelX, cancelY, btnW, btnH);
            r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b);
            r.DrawRect(cancelX, cancelY, btnW, btnH);
            {
                float tw = f.MeasureW(cancelText.c_str());
                r.SetDrawColor(s.textNormal.r, s.textNormal.g, s.textNormal.b);
                f.DrawText(r, cancelText.c_str(),
                           cancelX + (btnW - tw) * 0.5f,
                           cancelY + (btnH - f.GlyphH()) * 0.5f);
            }
        }

        // Dialog border
        r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b);
        r.DrawRect(dx, dy, dw, dh);
    }

private:
    float lastMx = -9999.f;
    float lastMy = -9999.f;
};

} // namespace Titan
