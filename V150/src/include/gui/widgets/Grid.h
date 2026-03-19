// Grid.h — Grid layout container.
// Arranges children in a fixed-column grid. Children are placed left-to-right,
// wrapping to the next row when the column count is reached.
// Cell size is either fixed (cellW/cellH) or computed from columns and widget width.
//
// Usage:
//   auto* g = panel->Add<Grid>(x, y, w, columns, cellH, spacing);
//   g->Add<Button>(...);  // auto-positioned in next cell
#pragma once
#include "../Widget.h"
#include "../TitanStyle.h"
#include "../TitanFont.h"

namespace Titan {

class Grid : public Widget {
public:
    int   columns  = 2;
    float cellH    = 24.f;   // row height
    float spacingX = 4.f;    // horizontal gap between cells
    float spacingY = 4.f;    // vertical gap between rows
    float padX     = 0.f;
    float padY     = 0.f;
    bool  stretchCells = true;  // force each cell to cellW derived from columns
    bool  autoH    = true;      // grow h to fit all rows
    bool  dirty    = true;

    bool  showBg   = false;

    Grid() = default;
    Grid(float x, float y, float w,
         int columns = 2, float cellH = 24.f,
         float spacingX = 4.f, float spacingY = 4.f,
         float padX = 0.f, float padY = 0.f)
        : columns(columns), cellH(cellH),
          spacingX(spacingX), spacingY(spacingY),
          padX(padX), padY(padY)
    { this->x=x; this->y=y; this->w=w; this->h=0; }

    void MarkDirty() { dirty = true; }

    float CellWidth() const
    {
        int cols = columns > 0 ? columns : 1;
        float totalSpacing = spacingX * (float)(cols - 1);
        return (w - padX * 2.f - totalSpacing) / (float)cols;
    }

    void Layout()
    {
        if (!dirty) return;
        dirty = false;

        float cw = CellWidth();
        int col = 0;
        float cx = padX;
        float cy = padY;

        for (auto& child : children) {
            if (!child->visible) continue;

            child->x = cx;
            child->y = cy;
            if (stretchCells) child->w = cw;
            child->h = cellH;

            col++;
            if (col >= columns) {
                col = 0;
                cx  = padX;
                cy += cellH + spacingY;
            } else {
                cx += cw + spacingX;
            }
        }

        if (autoH) {
            int rows = ((int)visibleCount() + columns - 1) / columns;
            h = rows > 0
                ? padY * 2.f + rows * cellH + (rows - 1) * spacingY
                : padY * 2.f;
        }
    }

    bool ProcessInput(float mx, float my, bool ldown, bool lclick, bool lrelease,
                      TitanUI* ui, float ox, float oy) override
    {
        if (!visible || !enabled) return false;
        Layout();
        float ax = ox+x, ay = oy+y;
        for (int i = (int)children.size()-1; i >= 0; i--) {
            if (children[i]->ProcessInput(mx, my, ldown, lclick, lrelease, ui, ax, ay))
                return true;
        }
        return false;
    }

    void Render(LightningEngine::Renderer& r, TitanFont& f, float ox, float oy) override
    {
        if (!visible) return;
        Layout();
        float ax = ox+x, ay = oy+y;

        if (showBg) {
            const Style& s = gStyle;
            r.SetDrawColor(s.panelBg.r, s.panelBg.g, s.panelBg.b);
            r.FillRect(ax, ay, w, h);
        }

        for (auto& child : children) child->Render(r, f, ax, ay);
    }

private:
    size_t visibleCount() const {
        size_t n = 0;
        for (auto& c : children) if (c->visible) n++;
        return n;
    }
};

} // namespace Titan
