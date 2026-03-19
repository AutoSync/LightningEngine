// NumericUpDown.h — Number input with decrement/increment buttons.
#pragma once
#include <string>
#include <functional>
#include <cfloat>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <SDL3/SDL.h>
#include "../Widget.h"
#include "../TitanStyle.h"
#include "../TitanFont.h"

namespace Titan {

class NumericUpDown : public Widget {
public:
    float  value    = 0.f;
    float  minV     = -FLT_MAX;
    float  maxV     =  FLT_MAX;
    float  step     = 1.f;
    int    decimals = 2;
    bool   integer  = false;

    std::function<void(float)> onChanged;

    static constexpr float kBtnW = 18.f;

    NumericUpDown() = default;
    NumericUpDown(float x, float y, float w, float h,
                  float val = 0.f, float minv = -FLT_MAX, float maxv = FLT_MAX,
                  float stp = 1.f, int dec = 2, bool intMode = false)
        : value(val), minV(minv), maxV(maxv), step(stp), decimals(dec), integer(intMode)
    { this->x=x; this->y=y; this->w=w; this->h=h; }

    void SetValue(float v)
    {
        float clamped = v < minV ? minV : (v > maxV ? maxV : v);
        if (integer) clamped = (float)(int)clamped;
        if (clamped != value) {
            value = clamped;
            if (onChanged) onChanged(value);
        }
        // Sync edit buffer
        formatValue(editBuf, sizeof(editBuf));
    }

    bool ProcessInput(float mx, float my, bool ldown, bool lclick, bool lrelease,
                      TitanUI* ui, float ox, float oy) override
    {
        if (!visible || !enabled) return false;
        float ax = ox+x, ay = oy+y;
        bool inWidget = mx >= ax && mx < ax+w && my >= ay && my < ay+h;

        // Mouse wheel inside widget
        if (inWidget && ui && GetUIScrollY(ui) != 0.f) {
            SetValue(value + GetUIScrollY(ui) * step);
            return true;
        }

        // Determine rects
        float btnMinX = ax;
        float btnPlusX = ax + w - kBtnW;
        float fieldX  = ax + kBtnW;
        float fieldW  = w - kBtnW * 2.f;

        bool inMinus = mx >= btnMinX  && mx < btnMinX + kBtnW && my >= ay && my < ay+h;
        bool inPlus  = mx >= btnPlusX && mx < btnPlusX + kBtnW && my >= ay && my < ay+h;
        bool inField = mx >= fieldX   && mx < fieldX + fieldW   && my >= ay && my < ay+h;

        if (lclick) {
            if (inMinus) {
                SetValue(value - step);
                hoverMinus = true;
                return true;
            }
            if (inPlus) {
                SetValue(value + step);
                hoverPlus = true;
                return true;
            }
            if (inField && !editing) {
                // Enter edit mode
                editing = true;
                formatValue(editBuf, sizeof(editBuf));
                SetUIFocus(ui, this);
                return true;
            }
            if (!inWidget && editing) {
                commitEdit();
                editing = false;
                SetUIFocus(ui, nullptr);
            }
        }

        // Hover tracking
        hoverMinus = inMinus;
        hoverPlus  = inPlus;

        return inWidget && (ldown || lclick);
    }

    void OnLostFocus() override
    {
        if (editing) { commitEdit(); editing = false; }
    }

    void ReceiveText(const std::string& txt, bool backspace) override
    {
        if (!editing) return;
        if (backspace) {
            if (!editBuf[0]) return;
            size_t len = SDL_strlen(editBuf);
            if (len > 0) editBuf[len-1] = '\0';
            return;
        }
        for (char c : txt) {
            size_t len = SDL_strlen(editBuf);
            if (len >= sizeof(editBuf)-1) break;
            // Allow digits
            if (c >= '0' && c <= '9') {
                editBuf[len] = c; editBuf[len+1] = '\0';
            }
            // Allow decimal point (if not integer and not already present)
            else if (c == '.' && !integer && !SDL_strchr(editBuf, '.')) {
                editBuf[len] = c; editBuf[len+1] = '\0';
            }
            // Allow leading minus sign (first char only)
            else if (c == '-' && len == 0) {
                editBuf[len] = c; editBuf[len+1] = '\0';
            }
            // Enter commits
            else if (c == '\r' || c == '\n') {
                commitEdit();
                editing = false;
            }
        }
    }

    void Render(LightningEngine::Renderer& r, TitanFont& f, float ox, float oy) override
    {
        if (!visible) return;
        float ax = ox+x, ay = oy+y;
        const Style& s = gStyle;

        float btnMinX  = ax;
        float btnPlusX = ax + w - kBtnW;
        float fieldX   = ax + kBtnW;
        float fieldW   = w - kBtnW * 2.f;

        // Field background
        r.SetDrawColor(s.btnNormal.r, s.btnNormal.g, s.btnNormal.b);
        r.FillRect(fieldX, ay, fieldW, h);

        // Minus button
        Color minusBg = hoverMinus ? s.btnHover : s.btnNormal;
        r.SetDrawColor(minusBg.r, minusBg.g, minusBg.b);
        r.FillRect(btnMinX, ay, kBtnW, h);

        // Plus button
        Color plusBg = hoverPlus ? s.btnHover : s.btnNormal;
        r.SetDrawColor(plusBg.r, plusBg.g, plusBg.b);
        r.FillRect(btnPlusX, ay, kBtnW, h);

        // Minus sign (horizontal bar)
        float midY = ay + h * 0.5f;
        r.SetDrawColor(s.textBright.r, s.textBright.g, s.textBright.b);
        r.FillRect(btnMinX + 4.f, midY - 1.f, kBtnW - 8.f, 2.f);

        // Plus sign (cross)
        float plusMidX = btnPlusX + kBtnW * 0.5f;
        r.FillRect(btnPlusX + 4.f, midY - 1.f, kBtnW - 8.f, 2.f);        // horizontal
        r.FillRect(plusMidX - 1.f, ay + 4.f,   2.f,          h - 8.f);   // vertical

        // Value text
        char buf[64];
        if (editing) {
            SDL_snprintf(buf, sizeof(buf), "%s", editBuf);
        } else {
            formatValue(buf, sizeof(buf));
        }

        float textW = f.MeasureW(buf);
        float textX = fieldX + (fieldW - textW) * 0.5f;
        float textY = ay + (h - f.GlyphH()) * 0.5f;

        r.SetDrawColor(s.textNormal.r, s.textNormal.g, s.textNormal.b);
        f.DrawText(r, buf, textX, textY);

        // Cursor blink when editing
        if (editing) {
            Uint64 t = SDL_GetTicks();
            if ((t / 500) % 2 == 0) {
                float cw = f.MeasureW(buf);
                r.SetDrawColor(s.highlight.r, s.highlight.g, s.highlight.b);
                r.FillRect(textX + cw + 1.f, textY, 1.5f, f.GlyphH());
            }
        }

        // Borders
        r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b);
        r.DrawRect(ax, ay, w, h);
        // Dividers between buttons and field
        r.FillRect(fieldX, ay, 1.f, h);
        r.FillRect(btnPlusX, ay, 1.f, h);
    }

private:
    bool  editing    = false;
    bool  hoverMinus = false;
    bool  hoverPlus  = false;
    char  editBuf[64] = {};

    void formatValue(char* buf, size_t sz) const
    {
        if (integer) {
            SDL_snprintf(buf, sz, "%d", (int)value);
        } else {
            char fmt[16];
            SDL_snprintf(fmt, sizeof(fmt), "%%.%df", decimals);
            SDL_snprintf(buf, sz, fmt, value);
        }
    }

    void commitEdit()
    {
        float parsed = (float)SDL_atof(editBuf);
        SetValue(parsed);
    }
};

} // namespace Titan
