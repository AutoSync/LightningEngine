// Text.h — Multi-line text widget with optional word wrap.
// Unlike Label (single line, no layout), Text handles newlines and wraps
// long lines to fit the widget width. Supports scrolling via scrollOffset.
#pragma once
#include <string>
#include <vector>
#include "../Widget.h"
#include "../TitanStyle.h"
#include "../TitanFont.h"

namespace Titan {

class Text : public Widget {
public:
    std::string text;
    bool  wordWrap  = true;
    bool  scrollable = false;   // allow vertical scroll when content exceeds h
    float scrollOffset = 0.f;

    // Colour
    Uint8 R=0, G=0, B=0;
    bool  useStyleColor = true;

    Text() = default;
    Text(float x, float y, float w, float h,
         const char* text = "", bool wordWrap = true)
        : text(text ? text : ""), wordWrap(wordWrap)
    { this->x=x; this->y=y; this->w=w; this->h=h; }

    void SetText(const char* t)  { text = t ? t : ""; }
    void SetColor(Uint8 r, Uint8 g, Uint8 b) { R=r; G=g; B=b; useStyleColor=false; }

    bool ProcessInput(float mx, float my, bool ldown, bool lclick, bool lrelease,
                      TitanUI* ui, float ox, float oy) override
    {
        if (!visible || !enabled) return false;
        if (scrollable && ui && GetUIScrollY(ui) != 0.f) {
            bool inView = mx >= ox+x && mx < ox+x+w && my >= oy+y && my < oy+y+h;
            if (inView) {
                scrollOffset -= GetUIScrollY(ui) * gStyle.lineH;
                if (scrollOffset < 0.f) scrollOffset = 0.f;
            }
        }
        return false;
    }

    void Render(LightningEngine::Renderer& r, TitanFont& f, float ox, float oy) override
    {
        if (!visible || text.empty()) return;
        const Style& s = gStyle;
        Color c = useStyleColor ? s.textNormal : Color{R, G, B};
        r.SetDrawColor(c.r, c.g, c.b);

        float ax = ox+x, ay = oy+y;
        float lineH = f.lineHeight > 0.f ? f.lineHeight : s.lineH;
        float curY  = ay - scrollOffset;

        // Split text into words and layout lines
        std::vector<std::string> lines = buildLines(f);

        for (const auto& line : lines) {
            if (curY + lineH >= ay && curY < ay + h) {
                f.DrawText(r, line.c_str(), ax, curY);
            }
            curY += lineH;
            if (curY > ay + h && !scrollable) break;
        }
    }

private:
    // Build display lines respecting '\n' and word wrap.
    std::vector<std::string> buildLines(TitanFont& f) const
    {
        std::vector<std::string> result;
        if (!wordWrap) {
            // Split only on '\n'
            std::string cur;
            for (char c : text) {
                if (c == '\n') { result.push_back(cur); cur.clear(); }
                else cur += c;
            }
            if (!cur.empty()) result.push_back(cur);
            return result;
        }

        // Word-wrap: split on spaces / newlines
        const float maxW = w;
        std::string cur;
        std::string word;

        auto flushWord = [&]() {
            if (word.empty()) return;
            std::string test = cur.empty() ? word : cur + " " + word;
            if (f.MeasureW(test.c_str()) <= maxW) {
                cur = test;
            } else {
                if (!cur.empty()) result.push_back(cur);
                // If word alone is too wide, hard-break it
                while (f.MeasureW(word.c_str()) > maxW && word.size() > 1) {
                    size_t split = word.size() - 1;
                    while (split > 0 && f.MeasureW(word.substr(0, split).c_str()) > maxW)
                        split--;
                    result.push_back(word.substr(0, split));
                    word = word.substr(split);
                }
                cur = word;
            }
            word.clear();
        };

        for (char c : text) {
            if (c == '\n') {
                flushWord();
                result.push_back(cur);
                cur.clear();
            } else if (c == ' ') {
                flushWord();
            } else {
                word += c;
            }
        }
        flushWord();
        if (!cur.empty()) result.push_back(cur);
        return result;
    }
};

} // namespace Titan
