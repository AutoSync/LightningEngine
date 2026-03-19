// TextField.h — Single-line text input with cursor, selection, and clipboard.
#pragma once
#include <functional>
#include <string>
#include <algorithm>
#include <SDL3/SDL.h>
#include "../Widget.h"
#include "../TitanStyle.h"
#include "../TitanFont.h"

namespace Titan {

class TextField : public Widget {
public:
    std::string text;
    std::string placeholder;
    bool        isFocused = false;

    std::function<void(const std::string&)> onChanged;

    TextField() = default;
    TextField(float x, float y, float w, float h, const char* ph = "")
        : placeholder(ph ? ph : "")
    { this->x=x; this->y=y; this->w=w; this->h=h; }

    void SetText(const std::string& t)
    {
        text      = t;
        cursorPos = (int)text.size();
        selAnchor = -1;
        scrollOff = 0;
    }

    bool ProcessInput(float mx, float my, bool ldown, bool lclick, bool lrelease,
                      TitanUI* ui, float ox, float oy) override
    {
        if (!visible || !enabled) return false;
        bool inside = Contains(mx, my, ox, oy);

        if (lclick) {
            if (inside) {
                if (!isFocused) { isFocused = true; SetUIFocus(ui, this); }
                bool shift = (SDL_GetModState() & SDL_KMOD_SHIFT) != 0;
                positionAtX(mx, ox + x + gStyle.padding, shift);
            } else if (isFocused) {
                isFocused = false;
                SetUIFocus(ui, nullptr);
            }
        }
        return inside && (ldown || lclick);
    }

    void OnLostFocus() override { isFocused = false; selAnchor = -1; }

    void ReceiveText(const std::string& txt, bool backspace) override
    {
        if (!isFocused) return;
        if (backspace) {
            if (!eraseSelection()) eraseChar(-1);
        } else if (!txt.empty()) {
            eraseSelection();
            text.insert(cursorPos, txt);
            cursorPos += (int)txt.size();
            notify();
        }
    }

    void ReceiveKey(SDL_Scancode key) override
    {
        if (!isFocused) return;
        SDL_Keymod mod   = SDL_GetModState();
        bool       ctrl  = (mod & SDL_KMOD_CTRL)  != 0;
        bool       shift = (mod & SDL_KMOD_SHIFT) != 0;

        switch (key) {
        case SDL_SCANCODE_LEFT:
            if (ctrl) wordStep(-1, shift); else moveCursor(-1, shift); break;
        case SDL_SCANCODE_RIGHT:
            if (ctrl) wordStep(+1, shift); else moveCursor(+1, shift); break;
        case SDL_SCANCODE_HOME:  setCursor(0,               shift); break;
        case SDL_SCANCODE_END:   setCursor((int)text.size(), shift); break;
        case SDL_SCANCODE_DELETE:
            if (!eraseSelection()) eraseChar(+1); break;
        case SDL_SCANCODE_A: if (ctrl) selectAll();                break;
        case SDL_SCANCODE_C: if (ctrl) copyToClipboard();         break;
        case SDL_SCANCODE_X: if (ctrl) { copyToClipboard(); eraseSelection(); notify(); } break;
        case SDL_SCANCODE_V: if (ctrl) pasteClipboard();          break;
        default: break;
        }
    }

    void Render(LightningEngine::Renderer& r, TitanFont& f, float ox, float oy) override
    {
        if (!visible) return;
        lastFont = &f;
        float ax  = ox+x, ay = oy+y;
        const Style& s   = gStyle;
        float pad    = s.padding;
        float fieldW = w - pad * 2.f;
        float ty     = ay + (h - f.GlyphH()) * 0.5f;
        float tx     = ax + pad;

        // Background
        r.SetDrawColor(isFocused ? 30 : 22, isFocused ? 35 : 22, isFocused ? 52 : 32);
        r.FillRect(ax, ay, w, h);

        // Border
        r.SetDrawColor(isFocused ? s.highlight.r : s.panelBorder.r,
                       isFocused ? s.highlight.g : s.panelBorder.g,
                       isFocused ? s.highlight.b : s.panelBorder.b);
        r.DrawRect(ax, ay, w, h);

        if (text.empty()) {
            if (!placeholder.empty()) {
                r.SetDrawColor(s.textDim.r, s.textDim.g, s.textDim.b);
                f.DrawText(r, placeholder.c_str(), tx, ty);
            }
            if (isFocused && blinkOn()) {
                r.SetDrawColor(s.highlight.r, s.highlight.g, s.highlight.b);
                r.FillRect(tx, ty, 1.5f, f.GlyphH());
            }
            return;
        }

        ensureVisible(f, fieldW);

        // Compute visible substring
        std::string vis = text.substr(scrollOff);
        while (!vis.empty() && f.MeasureW(vis.c_str()) > fieldW)
            vis.pop_back();
        int visEnd = scrollOff + (int)vis.size();

        // Selection highlight
        if (selAnchor >= 0 && selAnchor != cursorPos) {
            int lo = std::min(selAnchor, cursorPos);
            int hi = std::max(selAnchor, cursorPos);
            int vlo = std::max(lo, scrollOff);
            int vhi = std::min(hi, visEnd);
            if (vlo < vhi) {
                float x0 = tx + f.MeasureW(text.substr(scrollOff, vlo - scrollOff).c_str());
                float x1 = tx + f.MeasureW(text.substr(scrollOff, vhi - scrollOff).c_str());
                r.SetDrawColor(s.highlight.r, s.highlight.g, s.highlight.b, 90);
                r.FillRect(x0, ty - 1.f, x1 - x0, f.GlyphH() + 2.f);
            }
        }

        // Text
        r.SetDrawColor(s.textNormal.r, s.textNormal.g, s.textNormal.b);
        f.DrawText(r, vis.c_str(), tx, ty);

        // Cursor
        if (isFocused && blinkOn()) {
            int  cp   = std::clamp(cursorPos, scrollOff, visEnd);
            float coff = f.MeasureW(text.substr(scrollOff, cp - scrollOff).c_str());
            r.SetDrawColor(s.highlight.r, s.highlight.g, s.highlight.b);
            r.FillRect(tx + coff, ty, 1.5f, f.GlyphH());
        }
    }

private:
    int         cursorPos = 0;
    int         selAnchor = -1;
    int         scrollOff = 0;
    TitanFont*  lastFont  = nullptr;

    static bool blinkOn() { return (SDL_GetTicks() / 500) % 2 == 0; }

    void setCursor(int pos, bool extend)
    {
        pos = std::clamp(pos, 0, (int)text.size());
        if (extend) { if (selAnchor < 0) selAnchor = cursorPos; }
        else        { selAnchor = -1; }
        cursorPos = pos;
    }

    void moveCursor(int delta, bool extend)
    {
        if (!extend && selAnchor >= 0) {
            cursorPos = (delta < 0) ? std::min(cursorPos, selAnchor)
                                    : std::max(cursorPos, selAnchor);
            selAnchor = -1;
        } else {
            setCursor(cursorPos + delta, extend);
        }
    }

    void wordStep(int dir, bool extend)
    {
        int pos = cursorPos;
        if (dir < 0) {
            while (pos > 0 && text[pos-1] == ' ') pos--;
            while (pos > 0 && text[pos-1] != ' ') pos--;
        } else {
            while (pos < (int)text.size() && text[pos] == ' ') pos++;
            while (pos < (int)text.size() && text[pos] != ' ') pos++;
        }
        setCursor(pos, extend);
    }

    void selectAll() { selAnchor = 0; cursorPos = (int)text.size(); }

    bool hasSelection() const { return selAnchor >= 0 && selAnchor != cursorPos; }

    bool eraseSelection()
    {
        if (!hasSelection()) return false;
        int lo = std::min(selAnchor, cursorPos);
        int hi = std::max(selAnchor, cursorPos);
        text.erase(lo, hi - lo);
        cursorPos = lo;
        selAnchor = -1;
        notify();
        return true;
    }

    bool eraseChar(int dir)
    {
        if (dir < 0 && cursorPos > 0) {
            text.erase(--cursorPos, 1); notify(); return true;
        }
        if (dir > 0 && cursorPos < (int)text.size()) {
            text.erase(cursorPos, 1);  notify(); return true;
        }
        return false;
    }

    void copyToClipboard()
    {
        if (!hasSelection()) return;
        int lo = std::min(selAnchor, cursorPos);
        int hi = std::max(selAnchor, cursorPos);
        SDL_SetClipboardText(text.substr(lo, hi - lo).c_str());
    }

    void pasteClipboard()
    {
        if (!SDL_HasClipboardText()) return;
        char* cb = SDL_GetClipboardText();
        if (!cb) return;
        std::string clip = cb;
        SDL_free(cb);
        for (char& c : clip) if (c == '\n' || c == '\r') c = ' ';
        eraseSelection();
        text.insert(cursorPos, clip);
        cursorPos += (int)clip.size();
        notify();
    }

    void ensureVisible(TitanFont& f, float fieldW)
    {
        if (cursorPos < scrollOff) scrollOff = cursorPos;
        while (f.MeasureW(text.substr(scrollOff, cursorPos - scrollOff).c_str()) > fieldW)
            scrollOff++;
    }

    void positionAtX(float clickX, float fieldStartX, bool extend)
    {
        if (!lastFont) { setCursor((int)text.size(), extend); return; }
        std::string vis = text.substr(scrollOff);
        int pos = scrollOff;
        float prev = fieldStartX;
        for (int i = 1; i <= (int)vis.size(); i++) {
            float cur = fieldStartX + lastFont->MeasureW(vis.substr(0, i).c_str());
            if (cur > clickX) {
                pos = scrollOff + ((clickX - prev < cur - clickX) ? i-1 : i);
                setCursor(pos, extend);
                return;
            }
            prev = cur;
        }
        setCursor(scrollOff + (int)vis.size(), extend);
    }

    void notify() { if (onChanged) onChanged(text); }
};

} // namespace Titan
