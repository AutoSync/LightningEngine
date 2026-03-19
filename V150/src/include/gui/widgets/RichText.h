// RichText.h — Multi-line code editor widget.
// Supports C# syntax highlighting, line numbers, cursor navigation.
//
// Navigation (requires ReceiveKey forwarding from TitanUI):
//   Arrow keys — move cursor    Home/End — line start/end
//   Enter — insert newline      Tab — insert spaces (tabSize)
//   Backspace — delete left     Click — position cursor
//
// Usage:
//   auto* ed = panel->Add<RichText>(x, y, w, h);
//   ed->SetText("public class MyScript {\n    void Start() {}\n}");
//   ed->onChanged = [](auto& lines) { /* save */ };
#pragma once
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <cstring>
#include <SDL3/SDL.h>
#include "../Widget.h"
#include "../TitanStyle.h"
#include "../TitanFont.h"

namespace Titan {

class RichText : public Widget {
public:
    std::vector<std::string> lines = { "" };
    bool isFocused   = false;
    bool showLineNums = true;
    int  tabSize      = 4;

    enum class SyntaxMode { None, CSharp };
    SyntaxMode syntax = SyntaxMode::CSharp;

    std::function<void(const std::vector<std::string>&)> onChanged;

    // -----------------------------------------------------------------------
    RichText() = default;
    RichText(float x, float y, float w, float h)
    { this->x=x; this->y=y; this->w=w; this->h=h; }

    void SetText(const std::string& text)
    {
        lines.clear();
        std::string cur;
        for (char c : text) {
            if (c == '\n') { lines.push_back(cur); cur.clear(); }
            else cur += c;
        }
        lines.push_back(cur);
        curLine = 0; curCol = 0; scrollY = 0.f;
    }

    std::string GetText() const
    {
        std::string out;
        for (int i = 0; i < (int)lines.size(); i++) {
            if (i) out += '\n';
            out += lines[i];
        }
        return out;
    }

    // -----------------------------------------------------------------------
    // Focus
    // -----------------------------------------------------------------------
    void OnLostFocus() override { isFocused = false; }

    // -----------------------------------------------------------------------
    // Text input
    // -----------------------------------------------------------------------
    void ReceiveText(const std::string& txt, bool backspace) override
    {
        if (!isFocused) return;

        if (backspace) {
            if (curCol > 0) {
                lines[curLine].erase(curCol - 1, 1);
                curCol--;
            } else if (curLine > 0) {
                curCol = (int)lines[curLine - 1].size();
                lines[curLine - 1] += lines[curLine];
                lines.erase(lines.begin() + curLine);
                curLine--;
            }
            clampCursor();
            scrollToCursor();
            fire();
            return;
        }

        for (char c : txt) {
            if ((unsigned char)c < 32) continue; // skip control chars
            lines[curLine].insert(curCol, 1, c);
            curCol++;
        }
        scrollToCursor();
        fire();
    }

    // -----------------------------------------------------------------------
    // Navigation keys
    // -----------------------------------------------------------------------
    void ReceiveKey(SDL_Scancode key) override
    {
        if (!isFocused) return;
        switch (key) {
        case SDL_SCANCODE_LEFT:
            if (curCol > 0) curCol--;
            else if (curLine > 0) { curLine--; curCol = (int)lines[curLine].size(); }
            break;
        case SDL_SCANCODE_RIGHT:
            if (curCol < (int)lines[curLine].size()) curCol++;
            else if (curLine < (int)lines.size() - 1) { curLine++; curCol = 0; }
            break;
        case SDL_SCANCODE_UP:
            if (curLine > 0) { curLine--; curCol = std::min(curCol, (int)lines[curLine].size()); }
            break;
        case SDL_SCANCODE_DOWN:
            if (curLine < (int)lines.size() - 1) { curLine++; curCol = std::min(curCol, (int)lines[curLine].size()); }
            break;
        case SDL_SCANCODE_HOME:
            curCol = 0;
            break;
        case SDL_SCANCODE_END:
            curCol = (int)lines[curLine].size();
            break;
        case SDL_SCANCODE_RETURN:
        {
            std::string rest = lines[curLine].substr(curCol);
            lines[curLine]   = lines[curLine].substr(0, curCol);
            // Auto-indent: copy leading whitespace from current line
            std::string indent;
            for (char c : lines[curLine])
                if (c == ' ' || c == '\t') indent += c; else break;
            lines.insert(lines.begin() + curLine + 1, indent + rest);
            curLine++;
            curCol = (int)indent.size();
            fire();
            break;
        }
        case SDL_SCANCODE_TAB:
        {
            std::string sp(tabSize, ' ');
            lines[curLine].insert(curCol, sp);
            curCol += tabSize;
            fire();
            break;
        }
        case SDL_SCANCODE_PAGEUP:
            curLine = std::max(0, curLine - visibleLines());
            curCol  = std::min(curCol, (int)lines[curLine].size());
            break;
        case SDL_SCANCODE_PAGEDOWN:
            curLine = std::min((int)lines.size() - 1, curLine + visibleLines());
            curCol  = std::min(curCol, (int)lines[curLine].size());
            break;
        default: break;
        }
        clampCursor();
        scrollToCursor();
    }

    // -----------------------------------------------------------------------
    // Input
    // -----------------------------------------------------------------------
    bool ProcessInput(float mx, float my, bool ldown, bool lclick, bool lrelease,
                      TitanUI* ui, float ox, float oy) override
    {
        if (!visible || !enabled) return false;
        float ax = ox+x, ay = oy+y;
        bool inside = mx >= ax && mx < ax+w && my >= ay && my < ay+h;

        // Mouse wheel scroll
        if (inside && ui && GetUIScrollY(ui) != 0.f) {
            float lh = lineH();
            scrollY -= GetUIScrollY(ui) * lh * 3.f;
            clampScroll();
        }

        if (lclick) {
            if (inside) {
                if (!isFocused) { isFocused = true; SetUIFocus(ui, this); }
                // Position cursor from click
                float gw  = gutterW();
                float lh  = lineH();
                int clickLine = (int)((my - ay + scrollY) / lh);
                clickLine = std::clamp(clickLine, 0, (int)lines.size() - 1);
                curLine = clickLine;
                // Approximate column from X (use cached charW estimate)
                float relX = mx - ax - gw;
                curCol = (int)std::round(relX / estimCharW);
                curCol = std::clamp(curCol, 0, (int)lines[curLine].size());
                return true;
            } else if (isFocused) {
                isFocused = false;
                SetUIFocus(ui, nullptr);
            }
        }
        return inside && (ldown || lclick);
    }

    // -----------------------------------------------------------------------
    // Render
    // -----------------------------------------------------------------------
    void Render(LightningEngine::Renderer& r, TitanFont& f, float ox, float oy) override
    {
        if (!visible) return;
        cachedFont = &f;
        float ax = ox+x, ay = oy+y;
        const Style& s = gStyle;

        float lh = f.GlyphH() + 2.f;
        estimCharW = f.MeasureW("M") * 0.62f; // approximate mono char width

        // Background
        r.SetDrawColor(14, 14, 20);
        r.FillRect(ax, ay, w, h);

        // Gutter (line number background)
        float gw = showLineNums ? gutterWFromFont(f) : 0.f;
        r.SetDrawColor(20, 20, 30);
        r.FillRect(ax, ay, gw, h);

        // Gutter separator
        r.SetDrawColor(40, 40, 55);
        r.FillRect(ax + gw - 1.f, ay, 1.f, h);

        clampScroll();
        int firstLine = (int)(scrollY / lh);
        int lastLine  = firstLine + visibleLines() + 1;
        lastLine = std::min(lastLine, (int)lines.size());

        for (int li = firstLine; li < lastLine; li++) {
            float ry = ay + li * lh - scrollY;
            if (ry + lh < ay || ry > ay + h) continue;

            // Current line highlight
            if (li == curLine && isFocused) {
                r.SetDrawColor(25, 30, 45);
                r.FillRect(ax + gw, ry, w - gw, lh);
            }

            // Line number
            if (showLineNums) {
                char lnbuf[8];
                SDL_snprintf(lnbuf, sizeof(lnbuf), "%d", li + 1);
                float lnW = f.MeasureW(lnbuf);
                bool isCurrentLine = (li == curLine);
                if (isCurrentLine)
                    r.SetDrawColor(s.textNormal.r, s.textNormal.g, s.textNormal.b);
                else
                    r.SetDrawColor(55, 65, 85);
                f.DrawText(r, lnbuf, ax + gw - lnW - 6.f, ry + 1.f);
            }

            // Code line
            if (syntax == SyntaxMode::CSharp)
                renderLineCSharp(r, f, lines[li], ax + gw + 4.f, ry + 1.f, w - gw - 8.f, s);
            else {
                r.SetDrawColor(s.textNormal.r, s.textNormal.g, s.textNormal.b);
                f.DrawText(r, lines[li].c_str(), ax + gw + 4.f, ry + 1.f);
            }
        }

        // Cursor
        if (isFocused) {
            Uint64 t = SDL_GetTicks();
            if ((t / 530) % 2 == 0) {
                float gw2 = showLineNums ? gutterWFromFont(f) : 0.f;
                float cy  = ay + curLine * lh - scrollY + 1.f;
                std::string beforeCursor = lines[curLine].substr(0, curCol);
                float cx  = ax + gw2 + 4.f + f.MeasureW(beforeCursor.c_str());
                r.SetDrawColor(200, 200, 220);
                r.FillRect(cx, cy, 1.5f, lh - 2.f);
            }
        }

        // Border
        r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b);
        r.DrawRect(ax, ay, w, h);

        // Vertical scrollbar
        float totalH = (float)lines.size() * lh;
        if (totalH > h) {
            float sbW    = 7.f;
            float sbX    = ax + w - sbW;
            float ratio  = h / totalH;
            float thumbH = std::max(16.f, h * ratio);
            float thumbY = ay + (scrollY / (totalH - h)) * (h - thumbH);

            r.SetDrawColor(30, 35, 50);
            r.FillRect(sbX, ay, sbW, h);
            r.SetDrawColor(s.sliderThumb.r, s.sliderThumb.g, s.sliderThumb.b);
            r.FillRect(sbX + 1.f, thumbY, sbW - 2.f, thumbH);
        }
    }

private:
    int  curLine = 0, curCol = 0;
    float scrollY = 0.f;
    TitanFont* cachedFont = nullptr;
    float estimCharW = 7.f;

    float lineH() const
    {
        return cachedFont ? (cachedFont->GlyphH() + 2.f) : 16.f;
    }

    int visibleLines() const
    {
        return (int)(h / lineH()) + 1;
    }

    float gutterWFromFont(TitanFont& f) const
    {
        if (!showLineNums) return 0.f;
        int digits = 1;
        int n = (int)lines.size();
        while (n >= 10) { n /= 10; digits++; }
        char buf[8] = {};
        for (int i = 0; i < digits; i++) buf[i] = '9';
        return f.MeasureW(buf) + 16.f;
    }

    float gutterW() const
    {
        if (!showLineNums || !cachedFont) return 0.f;
        return gutterWFromFont(*cachedFont);
    }

    void clampCursor()
    {
        curLine = std::clamp(curLine, 0, (int)lines.size() - 1);
        curCol  = std::clamp(curCol,  0, (int)lines[curLine].size());
    }

    void clampScroll()
    {
        float lh   = lineH();
        float maxY = std::max(0.f, (float)lines.size() * lh - h);
        scrollY = std::clamp(scrollY, 0.f, maxY);
    }

    void scrollToCursor()
    {
        float lh     = lineH();
        float curY   = curLine * lh;
        float margin = lh * 2.f;
        if (curY < scrollY + margin)       scrollY = curY - margin;
        if (curY + lh > scrollY + h - margin) scrollY = curY + lh - h + margin;
        clampScroll();
    }

    void fire()
    {
        if (onChanged) onChanged(lines);
    }

    // -----------------------------------------------------------------------
    // C# syntax highlighting
    // -----------------------------------------------------------------------
    struct Span { std::string text; Uint8 r, g, b; };

    void renderLineCSharp(LightningEngine::Renderer& r, TitanFont& f,
                          const std::string& line,
                          float lx, float ly, float maxW,
                          const Style& s) const
    {
        auto spans = tokenizeCSharp(line);
        float cx = lx;
        for (auto& sp : spans) {
            if (cx > lx + maxW) break;
            r.SetDrawColor(sp.r, sp.g, sp.b);
            f.DrawText(r, sp.text.c_str(), cx, ly);
            cx += f.MeasureW(sp.text.c_str());
        }
    }

    std::vector<Span> tokenizeCSharp(const std::string& line) const
    {
        std::vector<Span> out;
        int n = (int)line.size();
        int i = 0;

        while (i < n) {
            // Single-line comment
            if (i + 1 < n && line[i] == '/' && line[i+1] == '/') {
                out.push_back({ line.substr(i), 85, 100, 115 });
                break;
            }
            // String literal
            if (line[i] == '"') {
                int j = i + 1;
                while (j < n && !(line[j] == '"' && line[j-1] != '\\')) j++;
                if (j < n) j++;
                out.push_back({ line.substr(i, j - i), 106, 170, 100 }); // green
                i = j;
                continue;
            }
            // Char literal
            if (line[i] == '\'') {
                int j = i + 1;
                while (j < n && !(line[j] == '\'' && line[j-1] != '\\')) j++;
                if (j < n) j++;
                out.push_back({ line.substr(i, j - i), 106, 170, 100 });
                i = j;
                continue;
            }
            // Number
            if (isdigit((unsigned char)line[i]) ||
                (line[i] == '.' && i+1 < n && isdigit((unsigned char)line[i+1])))
            {
                int j = i;
                while (j < n && (isdigit((unsigned char)line[j]) || line[j] == '.' ||
                                  line[j] == 'f' || line[j] == 'L' || line[j] == 'u'))
                    j++;
                out.push_back({ line.substr(i, j - i), 205, 140, 80 }); // orange
                i = j;
                continue;
            }
            // Identifier / keyword
            if (isalpha((unsigned char)line[i]) || line[i] == '_') {
                int j = i;
                while (j < n && (isalnum((unsigned char)line[j]) || line[j] == '_')) j++;
                std::string word = line.substr(i, j - i);
                Span sp;
                sp.text = word;
                if      (isCSharpKeyword(word)) { sp.r=86;  sp.g=156; sp.b=214; } // blue
                else if (isCSharpType(word))    { sp.r=78;  sp.g=201; sp.b=176; } // teal
                else                             { sp.r=212; sp.g=212; sp.b=212; } // normal
                out.push_back(sp);
                i = j;
                continue;
            }
            // Punctuation / operator — emit as-is
            out.push_back({ std::string(1, line[i]), 180, 180, 195 });
            i++;
        }
        return out;
    }

    static bool isCSharpKeyword(const std::string& w)
    {
        static const char* kw[] = {
            "public","private","protected","internal","static","void","class",
            "interface","struct","enum","if","else","for","foreach","while","do",
            "switch","case","break","continue","return","new","this","base",
            "using","namespace","try","catch","finally","throw","async","await",
            "override","virtual","abstract","sealed","readonly","ref","out","in",
            "get","set","value","true","false","null","is","as","typeof","sizeof",
            "checked","unchecked","lock","unsafe","fixed","const","event","delegate",
            "implicit","explicit","operator","params","partial",
            nullptr
        };
        for (int i = 0; kw[i]; i++)
            if (w == kw[i]) return true;
        return false;
    }

    static bool isCSharpType(const std::string& w)
    {
        static const char* types[] = {
            "int","float","double","bool","string","char","long","short",
            "byte","uint","ushort","ulong","object","decimal","sbyte","nint","nuint",
            "var","dynamic",
            nullptr
        };
        for (int i = 0; types[i]; i++)
            if (w == types[i]) return true;
        return false;
    }
};

} // namespace Titan
