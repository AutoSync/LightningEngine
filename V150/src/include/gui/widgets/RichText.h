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
#include <cctype>
#include <cmath>
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

    enum class SyntaxMode { None, CSharp, GLSL, JSON };
    SyntaxMode syntax = SyntaxMode::CSharp;
    bool showWhitespace = true;

    enum class LintSeverity { Info, Warning, Error };
    struct LintDiagnostic {
        int line = 0;
        LintSeverity severity = LintSeverity::Info;
        std::string message;
    };

    struct LintCounters {
        int infos = 0;
        int warnings = 0;
        int errors = 0;
    };

    std::function<void(const std::vector<std::string>&)> onChanged;
    std::function<void(float)> onFontScaleChanged;

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
        rebuildDiagnostics();
    }

    void SetFontScale(float scale)
    {
        float clamped = std::clamp(scale, 0.75f, 2.5f);
        if (std::fabs(clamped - fontScale) < 0.001f) return;
        fontScale = clamped;
        clampScroll();
        scrollToCursor();
        if (onFontScaleChanged) onFontScaleChanged(fontScale);
    }

    float GetFontScale() const { return fontScale; }

    LintCounters GetLintCounters() const
    {
        LintCounters counters;
        for (const auto& d : diagnostics) {
            if (d.severity == LintSeverity::Error) counters.errors++;
            else if (d.severity == LintSeverity::Warning) counters.warnings++;
            else counters.infos++;
        }
        return counters;
    }

    const std::vector<LintDiagnostic>& GetDiagnostics() const { return diagnostics; }

    void MoveCursorToLine(int line)
    {
        if (lines.empty()) return;
        curLine = std::clamp(line, 0, (int)lines.size() - 1);
        curCol = std::min(curCol, (int)lines[curLine].size());
        scrollToCursor();
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
            const SDL_Keymod mods = SDL_GetModState();
            const bool ctrlDown = (mods & SDL_KMOD_CTRL) != 0;
            if (ctrlDown) {
                SetFontScale(fontScale + GetUIScrollY(ui) * 0.08f);
                return true;
            } else {
                float lh = lineH();
                scrollY -= GetUIScrollY(ui) * lh * 3.f;
                clampScroll();
            }
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
        lh = lineH();
        estimCharW = measureTextW(f, "M") * 0.62f; // approximate mono char width

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
                float lnW = measureTextW(f, lnbuf);
                bool isCurrentLine = (li == curLine);
                if (isCurrentLine)
                    r.SetDrawColor(s.textNormal.r, s.textNormal.g, s.textNormal.b);
                else
                    r.SetDrawColor(55, 65, 85);
                drawText(r, f, lnbuf, ax + gw - lnW - 6.f, ry + 1.f);
            }

            // Indentation guide (4 spaces step)
            int leadingSpaces = 0;
            for (char c : lines[li]) {
                if (c == ' ') leadingSpaces++;
                else if (c == '\t') leadingSpaces += tabSize;
                else break;
            }
            int guides = leadingSpaces / tabSize;
            float guideStep = estimCharW * (float)tabSize;
            for (int gi = 1; gi <= guides; ++gi) {
                float gx = ax + gw + 4.f + guideStep * (float)gi;
                r.SetDrawColor(42, 46, 60, 140);
                r.FillRect(gx, ry + 1.f, 1.f, lh - 2.f);
            }

            // Code line
            if (syntax == SyntaxMode::CSharp)
                renderLineCSharp(r, f, lines[li], ax + gw + 4.f, ry + 1.f, w - gw - 8.f, s);
            else if (syntax == SyntaxMode::GLSL)
                renderLineGLSL(r, f, lines[li], ax + gw + 4.f, ry + 1.f, w - gw - 8.f, s);
            else if (syntax == SyntaxMode::JSON)
                renderLineJSON(r, f, lines[li], ax + gw + 4.f, ry + 1.f, w - gw - 8.f, s);
            else {
                r.SetDrawColor(s.textNormal.r, s.textNormal.g, s.textNormal.b);
                drawText(r, f, lines[li].c_str(), ax + gw + 4.f, ry + 1.f);
            }

            if (showWhitespace) {
                renderWhitespaceHints(r, f, lines[li], ax + gw + 4.f, ry + 1.f, s);
            }

            // Lint marker in gutter (most severe diagnostic for the line)
            LintSeverity sev;
            if (tryLineSeverity(li, sev) && showLineNums) {
                if (sev == LintSeverity::Error)      r.SetDrawColor(220, 80, 80);
                else if (sev == LintSeverity::Warning) r.SetDrawColor(225, 175, 70);
                else                                  r.SetDrawColor(90, 160, 220);
                r.FillRect(ax + gw - 4.f, ry + (lh - 5.f) * 0.5f, 3.f, 5.f);
            }
        }

        // Cursor
        if (isFocused) {
            Uint64 t = SDL_GetTicks();
            if ((t / 530) % 2 == 0) {
                float gw2 = showLineNums ? gutterWFromFont(f) : 0.f;
                float cy  = ay + curLine * lh - scrollY + 1.f;
                std::string beforeCursor = lines[curLine].substr(0, curCol);
                float cx  = ax + gw2 + 4.f + measureTextW(f, beforeCursor.c_str());
                r.SetDrawColor(200, 200, 220);
                r.FillRect(cx, cy, 1.5f, lh - 2.f);
            }

            renderBracketMatch(r, f, ax + gw + 4.f, ay + 1.f, lh);
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

        // Lint counters in bottom-right corner
        if (!diagnostics.empty()) {
            LintCounters c = GetLintCounters();
            char lintBuf[64];
            SDL_snprintf(lintBuf, sizeof(lintBuf), "E:%d  W:%d  I:%d", c.errors, c.warnings, c.infos);
            float tw = measureTextW(f, lintBuf);
            r.SetDrawColor(14, 14, 20, 210);
            r.FillRect(ax + w - tw - 10.f, ay + h - lh - 4.f, tw + 6.f, lh + 2.f);
            r.SetDrawColor(150, 155, 170);
            drawText(r, f, lintBuf, ax + w - tw - 7.f, ay + h - lh - 3.f);
        }
    }

private:
    int  curLine = 0, curCol = 0;
    float scrollY = 0.f;
    TitanFont* cachedFont = nullptr;
    float estimCharW = 7.f;
    float fontScale = 1.f;
    std::vector<LintDiagnostic> diagnostics;

    float lineH() const
    {
        return cachedFont ? ((cachedFont->GlyphH() + 2.f) * fontScale) : (16.f * fontScale);
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
        return measureTextW(f, buf) + 16.f;
    }

    float measureTextW(TitanFont& f, const char* text) const
    {
        return f.MeasureW(text) * fontScale;
    }

    void drawText(LightningEngine::Renderer& r, TitanFont& f,
                  const char* text, float x, float y) const
    {
        if (!text || !*text) return;
        if (std::fabs(fontScale - 1.f) < 0.01f) {
            f.DrawText(r, text, x, y);
            return;
        }

        float cx = x;
        float baseY = y + f.baseline * fontScale;
        const float invW = (float)f.atlasW > 0.f ? 1.f / (float)f.atlasW : 0.f;
        const float invH = (float)f.atlasH > 0.f ? 1.f / (float)f.atlasH : 0.f;

        while (*text) {
            char c = *text++;
            if (c == '\n') {
                cx = x;
                baseY += f.lineHeight * fontScale;
                continue;
            }
            if (c < 32 || c > 127) c = '?';

            const stbtt_bakedchar& g = f.glyphs[(int)c - 32];
            float x0 = cx + g.xoff * fontScale;
            float y0 = baseY + g.yoff * fontScale;
            float gw = (float)(g.x1 - g.x0) * fontScale;
            float gh = (float)(g.y1 - g.y0) * fontScale;
            float u0 = (float)g.x0 * invW;
            float v0 = (float)g.y0 * invH;
            float u1 = (float)g.x1 * invW;
            float v1 = (float)g.y1 * invH;

            r.DrawTextureRegion(const_cast<LightningEngine::Texture&>(f.atlas),
                               x0, y0, gw, gh, u0, v0, u1, v1);
            cx += g.xadvance * fontScale;
        }
    }

    bool tryLineSeverity(int line, LintSeverity& out) const
    {
        bool found = false;
        LintSeverity best = LintSeverity::Info;
        for (const auto& d : diagnostics) {
            if (d.line != line) continue;
            if (!found || d.severity == LintSeverity::Error ||
                (d.severity == LintSeverity::Warning && best == LintSeverity::Info)) {
                best = d.severity;
                found = true;
            }
        }
        if (found) out = best;
        return found;
    }

    void pushLint(int line, LintSeverity severity, const std::string& message)
    {
        diagnostics.push_back({ line, severity, message });
    }

    void rebuildDiagnostics()
    {
        diagnostics.clear();
        if (lines.empty()) return;

        int braceBalance = 0;
        for (int li = 0; li < (int)lines.size(); ++li) {
            const std::string& ln = lines[li];

            if (!ln.empty() && (ln.back() == ' ' || ln.back() == '\t'))
                pushLint(li, LintSeverity::Warning, "Trailing whitespace");

            if (ln.find('\t') != std::string::npos)
                pushLint(li, LintSeverity::Info, "Tabs mixed with spaces");

            if ((int)ln.size() > 120)
                pushLint(li, LintSeverity::Warning, "Line exceeds 120 columns");

            if (syntax == SyntaxMode::CSharp) {
                if (ln.find("TODO") != std::string::npos || ln.find("FIXME") != std::string::npos)
                    pushLint(li, LintSeverity::Info, "Pending todo/fixme");
            }

            for (char c : ln) {
                if (c == '{') braceBalance++;
                else if (c == '}') {
                    braceBalance--;
                    if (braceBalance < 0) {
                        pushLint(li, LintSeverity::Error, "Closing brace without opening brace");
                        braceBalance = 0;
                    }
                }
            }
        }

        if (braceBalance > 0) {
            pushLint((int)lines.size() - 1, LintSeverity::Error, "Unclosed block: missing '}'");
        }
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
        rebuildDiagnostics();
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
            drawText(r, f, sp.text.c_str(), cx, ly);
            cx += measureTextW(f, sp.text.c_str());
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

    void renderLineGLSL(LightningEngine::Renderer& r, TitanFont& f,
                        const std::string& line,
                        float lx, float ly, float maxW,
                        const Style& s) const
    {
        auto spans = tokenizeGLSL(line);
        float cx = lx;
        for (auto& sp : spans) {
            if (cx > lx + maxW) break;
            r.SetDrawColor(sp.r, sp.g, sp.b);
            drawText(r, f, sp.text.c_str(), cx, ly);
            cx += measureTextW(f, sp.text.c_str());
        }
    }

    std::vector<Span> tokenizeGLSL(const std::string& line) const
    {
        std::vector<Span> out;
        int n = (int)line.size();
        int i = 0;

        while (i < n) {
            if (i + 1 < n && line[i] == '/' && line[i + 1] == '/') {
                out.push_back({ line.substr(i), 85, 100, 115 });
                break;
            }

            if (line[i] == '#') {
                out.push_back({ line.substr(i), 165, 120, 220 });
                break;
            }

            if (line[i] == '"') {
                int j = i + 1;
                while (j < n && !(line[j] == '"' && line[j - 1] != '\\')) j++;
                if (j < n) j++;
                out.push_back({ line.substr(i, j - i), 106, 170, 100 });
                i = j;
                continue;
            }

            if (std::isdigit((unsigned char)line[i]) ||
                (line[i] == '.' && i + 1 < n && std::isdigit((unsigned char)line[i + 1])))
            {
                int j = i;
                while (j < n && (std::isdigit((unsigned char)line[j]) || line[j] == '.' ||
                                 line[j] == 'f' || line[j] == 'u'))
                    j++;
                out.push_back({ line.substr(i, j - i), 205, 140, 80 });
                i = j;
                continue;
            }

            if (std::isalpha((unsigned char)line[i]) || line[i] == '_') {
                int j = i;
                while (j < n && (std::isalnum((unsigned char)line[j]) || line[j] == '_')) j++;
                std::string word = line.substr(i, j - i);
                if (isGLSLKeyword(word)) out.push_back({ word, 86, 156, 214 });
                else if (isGLSLType(word)) out.push_back({ word, 78, 201, 176 });
                else out.push_back({ word, 212, 212, 212 });
                i = j;
                continue;
            }

            out.push_back({ std::string(1, line[i]), 180, 180, 195 });
            i++;
        }

        return out;
    }

    void renderLineJSON(LightningEngine::Renderer& r, TitanFont& f,
                        const std::string& line,
                        float lx, float ly, float maxW,
                        const Style& s) const
    {
        auto spans = tokenizeJSON(line);
        float cx = lx;
        for (auto& sp : spans) {
            if (cx > lx + maxW) break;
            r.SetDrawColor(sp.r, sp.g, sp.b);
            drawText(r, f, sp.text.c_str(), cx, ly);
            cx += measureTextW(f, sp.text.c_str());
        }
    }

    std::vector<Span> tokenizeJSON(const std::string& line) const
    {
        std::vector<Span> out;
        int n = (int)line.size();
        int i = 0;

        while (i < n) {
            if (std::isspace((unsigned char)line[i])) {
                int j = i;
                while (j < n && std::isspace((unsigned char)line[j])) j++;
                out.push_back({ line.substr(i, j - i), 180, 180, 195 });
                i = j;
                continue;
            }

            if (line[i] == '"') {
                int j = i + 1;
                while (j < n && !(line[j] == '"' && line[j - 1] != '\\')) j++;
                if (j < n) j++;
                std::string token = line.substr(i, j - i);

                int k = j;
                while (k < n && std::isspace((unsigned char)line[k])) k++;
                bool isKey = (k < n && line[k] == ':');
                if (isKey) out.push_back({ token, 120, 190, 240 });
                else out.push_back({ token, 106, 170, 100 });

                i = j;
                continue;
            }

            if (std::isdigit((unsigned char)line[i]) || line[i] == '-') {
                int j = i + 1;
                while (j < n && (std::isdigit((unsigned char)line[j]) || line[j] == '.' ||
                                 line[j] == 'e' || line[j] == 'E' || line[j] == '+' || line[j] == '-'))
                    j++;
                out.push_back({ line.substr(i, j - i), 205, 140, 80 });
                i = j;
                continue;
            }

            if (std::isalpha((unsigned char)line[i])) {
                int j = i;
                while (j < n && std::isalpha((unsigned char)line[j])) j++;
                std::string word = line.substr(i, j - i);
                if (word == "true" || word == "false" || word == "null")
                    out.push_back({ word, 165, 120, 220 });
                else
                    out.push_back({ word, 212, 212, 212 });
                i = j;
                continue;
            }

            out.push_back({ std::string(1, line[i]), 180, 180, 195 });
            i++;
        }

        return out;
    }

    static bool isGLSLKeyword(const std::string& w)
    {
        static const char* kw[] = {
            "if","else","for","while","do","switch","case","break","continue","return",
            "in","out","inout","uniform","layout","const","precision","discard",
            nullptr
        };
        for (int i = 0; kw[i]; i++)
            if (w == kw[i]) return true;
        return false;
    }

    static bool isGLSLType(const std::string& w)
    {
        static const char* types[] = {
            "void","bool","int","uint","float","double",
            "vec2","vec3","vec4","ivec2","ivec3","ivec4",
            "uvec2","uvec3","uvec4","mat2","mat3","mat4",
            "sampler2D","samplerCube",
            nullptr
        };
        for (int i = 0; types[i]; i++)
            if (w == types[i]) return true;
        return false;
    }

    void renderWhitespaceHints(LightningEngine::Renderer& r, TitanFont& f,
                               const std::string& line, float lx, float ly,
                               const Style& s) const
    {
        r.SetDrawColor(95, 100, 120, 120);

        float x = lx;
        for (char c : line) {
            if (c == ' ') {
                drawText(r, f, ".", x, ly);
                x += measureTextW(f, " ");
            } else if (c == '\t') {
                drawText(r, f, ">", x, ly);
                x += measureTextW(f, " ") * (float)tabSize;
            } else {
                std::string one(1, c);
                x += measureTextW(f, one.c_str());
            }
        }

        // end-of-line marker
        drawText(r, f, "$", x, ly);
    }

    bool isBracketChar(char c) const
    {
        return c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']';
    }

    bool isBracketPair(char open, char close) const
    {
        return (open == '(' && close == ')') ||
               (open == '{' && close == '}') ||
               (open == '[' && close == ']');
    }

    bool findMatchingBracket(int line, int col, int& outLine, int& outCol) const
    {
        if (line < 0 || line >= (int)lines.size() || col < 0 || col >= (int)lines[line].size()) return false;

        char b = lines[line][col];
        if (!isBracketChar(b)) return false;

        if (b == '(' || b == '{' || b == '[') {
            int depth = 0;
            for (int li = line; li < (int)lines.size(); ++li) {
                int start = (li == line) ? col : 0;
                for (int ci = start; ci < (int)lines[li].size(); ++ci) {
                    char c = lines[li][ci];
                    if (c == b) depth++;
                    else if (isBracketPair(b, c)) {
                        depth--;
                        if (depth == 0) {
                            outLine = li;
                            outCol = ci;
                            return true;
                        }
                    }
                }
            }
            return false;
        }

        char open = (b == ')') ? '(' : (b == '}') ? '{' : '[';
        int depth = 0;
        for (int li = line; li >= 0; --li) {
            int start = (li == line) ? col : ((int)lines[li].size() - 1);
            for (int ci = start; ci >= 0; --ci) {
                char c = lines[li][ci];
                if (c == b) depth++;
                else if (c == open) {
                    depth--;
                    if (depth == 0) {
                        outLine = li;
                        outCol = ci;
                        return true;
                    }
                }
            }
        }
        return false;
    }

    void renderBracketHighlightAt(LightningEngine::Renderer& r, TitanFont& f,
                                  int line, int col, float textX, float textY,
                                  float lh) const
    {
        if (line < 0 || line >= (int)lines.size() || col < 0 || col >= (int)lines[line].size()) return;

        float y = textY + (float)line * lh - scrollY;
        if (y + lh < textY || y > textY + h) return;

        std::string before = lines[line].substr(0, col);
        std::string one(1, lines[line][col]);
        float x = textX + measureTextW(f, before.c_str());
        float wch = std::max(6.f, measureTextW(f, one.c_str()));

        r.SetDrawColor(80, 95, 130, 110);
        r.FillRect(x - 1.f, y, wch + 2.f, lh);
    }

    void renderBracketMatch(LightningEngine::Renderer& r, TitanFont& f,
                            float textX, float textY, float lh) const
    {
        if (curLine < 0 || curLine >= (int)lines.size()) return;

        int srcCol = -1;
        if (curCol < (int)lines[curLine].size() && isBracketChar(lines[curLine][curCol]))
            srcCol = curCol;
        else if (curCol > 0 && isBracketChar(lines[curLine][curCol - 1]))
            srcCol = curCol - 1;

        if (srcCol < 0) return;

        int dstLine = -1, dstCol = -1;
        if (!findMatchingBracket(curLine, srcCol, dstLine, dstCol)) return;

        renderBracketHighlightAt(r, f, curLine, srcCol, textX, textY, lh);
        renderBracketHighlightAt(r, f, dstLine, dstCol, textX, textY, lh);
    }
};

} // namespace Titan
