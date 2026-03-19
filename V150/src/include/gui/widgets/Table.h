// Table.h — Scrollable data table with header row, column widths, and row selection.
#pragma once
#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include "../Widget.h"
#include "../TitanStyle.h"
#include "../TitanFont.h"

namespace Titan {

class Table : public Widget {
public:
    struct Column {
        std::string header;
        float       width = 80.f;
    };
    using Row = std::vector<std::string>;

    std::vector<Column>      columns;
    std::vector<Row>         rows;
    int                      selectedRow = -1;

    std::function<void(int)> onSelect;

    static constexpr float kRowH   = 20.f;
    static constexpr float kHeadH  = 22.f;

    Table() = default;
    Table(float x, float y, float w, float h)
    { this->x=x; this->y=y; this->w=w; this->h=h; }

    void AddColumn(const char* header, float colW = 80.f)
    { columns.push_back({ header ? header : "", colW }); }

    void AddRow(const std::vector<std::string>& row) { rows.push_back(row); }

    void Clear() { rows.clear(); selectedRow = -1; scrollY = 0.f; }

    bool ProcessInput(float mx, float my, bool ldown, bool lclick, bool lrelease,
                      TitanUI* ui, float ox, float oy) override
    {
        if (!visible || !enabled) return false;
        float ax = ox+x, ay = oy+y;
        if (!Contains(mx, my, ox, oy)) return false;

        // Mouse wheel scroll
        float wheel = GetUIScrollY(ui);
        if (wheel != 0.f) {
            scrollY -= wheel * kRowH * 3.f;
            clampScroll();
            return true;
        }

        // Click row selection (content area below header)
        float contentY = ay + kHeadH;
        if (lclick && my >= contentY) {
            int row = (int)((my - contentY + scrollY) / kRowH);
            if (row >= 0 && row < (int)rows.size()) {
                selectedRow = row;
                if (onSelect) onSelect(selectedRow);
            }
        }
        return ldown || lclick;
    }

    void Render(LightningEngine::Renderer& r, TitanFont& f, float ox, float oy) override
    {
        if (!visible) return;
        float ax = ox+x, ay = oy+y;
        const Style& s = gStyle;

        // Background
        r.SetDrawColor(s.panelBg.r, s.panelBg.g, s.panelBg.b);
        r.FillRect(ax, ay, w, h);

        // Header background
        r.SetDrawColor(s.btnNormal.r, s.btnNormal.g, s.btnNormal.b);
        r.FillRect(ax, ay, w, kHeadH);

        // Draw column headers + vertical dividers
        float cx = ax;
        for (int ci = 0; ci < (int)columns.size(); ci++) {
            const auto& col = columns[ci];
            float cw = col.width;
            if (cx + cw > ax + w) cw = ax + w - cx;

            // Header text
            float th = f.MeasureW(col.header.c_str());
            r.SetDrawColor(s.textBright.r, s.textBright.g, s.textBright.b);
            f.DrawText(r, col.header.c_str(),
                       cx + (cw - th) * 0.5f,
                       ay + (kHeadH - f.GlyphH()) * 0.5f);

            // Column divider
            if (ci < (int)columns.size() - 1) {
                r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b);
                r.FillRect(cx + cw, ay, 1.f, h);
            }
            cx += cw;
            if (cx >= ax + w) break;
        }

        // Header bottom border
        r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b);
        r.FillRect(ax, ay + kHeadH, w, 1.f);

        // Content area (rows)
        float contentH = h - kHeadH;
        float contentY = ay + kHeadH;

        int firstRow = (int)(scrollY / kRowH);
        int maxRows  = (int)(contentH / kRowH) + 2;

        for (int ri = firstRow; ri < std::min(firstRow + maxRows, (int)rows.size()); ri++) {
            float ry = contentY + ri * kRowH - scrollY;
            if (ry + kRowH < contentY) continue;
            if (ry >= contentY + contentH) break;

            // Row background
            bool selected = (ri == selectedRow);
            if (selected) {
                r.SetDrawColor(s.highlight.r, s.highlight.g, s.highlight.b, 60);
                r.FillRect(ax, ry, w, kRowH);
            } else if (ri % 2 == 1) {
                r.SetDrawColor(s.panelBg.r + 4, s.panelBg.g + 4, s.panelBg.b + 4);
                r.FillRect(ax, ry, w, kRowH);
            }

            // Cell text
            float cx2 = ax;
            float pad  = gStyle.padding;
            for (int ci = 0; ci < (int)columns.size() && ci < (int)rows[ri].size(); ci++) {
                float cw = columns[ci].width;
                if (cx2 + cw > ax + w) cw = ax + w - cx2;
                const std::string& cell = rows[ri][ci];

                // Clip cell text to column width
                std::string txt = cell;
                while (!txt.empty() && f.MeasureW(txt.c_str()) > cw - pad * 2.f)
                    txt.pop_back();

                r.SetDrawColor(selected ? s.textBright.r : s.textNormal.r,
                               selected ? s.textBright.g : s.textNormal.g,
                               selected ? s.textBright.b : s.textNormal.b);
                f.DrawText(r, txt.c_str(), cx2 + pad, ry + (kRowH - f.GlyphH()) * 0.5f);
                cx2 += cw;
                if (cx2 >= ax + w) break;
            }

            // Row bottom border
            r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b, 40);
            r.FillRect(ax, ry + kRowH - 1.f, w, 1.f);
        }

        // Outer border
        r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b);
        r.DrawRect(ax, ay, w, h);

        // Scrollbar (if content taller than area)
        float totalH = rows.size() * kRowH;
        if (totalH > contentH) {
            float thumbH = std::max(16.f, contentH * contentH / totalH);
            float thumbY = contentY + (scrollY / (totalH - contentH)) * (contentH - thumbH);
            r.SetDrawColor(s.panelBorder.r + 20, s.panelBorder.g + 20, s.panelBorder.b + 20);
            r.FillRect(ax + w - 6.f, thumbY, 4.f, thumbH);
        }
    }

private:
    float scrollY = 0.f;

    void clampScroll()
    {
        float contentH = h - kHeadH;
        float totalH   = rows.size() * kRowH;
        float maxScroll = std::max(0.f, totalH - contentH);
        scrollY = std::clamp(scrollY, 0.f, maxScroll);
    }
};

} // namespace Titan
