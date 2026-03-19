// Chart.h — Line and bar chart widget with auto-scaling, grid, and legend.
#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include "../Widget.h"
#include "../TitanStyle.h"
#include "../TitanFont.h"

namespace Titan {

class Chart : public Widget {
public:
    enum class Type { Line, Bar };

    struct Series {
        std::string        name;
        Uint8              r = 100, g = 180, b = 255;
        std::vector<float> values;
    };

    Type        type       = Type::Line;
    std::string titleStr;
    bool        showGrid   = true;
    bool        showLegend = true;
    float       yMin       = 0.f;   // 0,0 = auto-scale from data
    float       yMax       = 0.f;
    float       lineThick  = 1.5f;

    std::vector<Series> series;

    Chart() = default;
    Chart(float x, float y, float w, float h, Type t = Type::Line)
        : type(t)
    { this->x=x; this->y=y; this->w=w; this->h=h; }

    Series& AddSeries(const char* name, Uint8 r=100, Uint8 g=180, Uint8 b=255)
    {
        series.push_back({ name ? name : "", r, g, b });
        return series.back();
    }

    void SetData(int idx, const std::vector<float>& data)
    {
        if (idx >= 0 && idx < (int)series.size())
            series[idx].values = data;
    }

    void ClearData()
    {
        for (auto& s : series) s.values.clear();
    }

    bool ProcessInput(float mx, float my, bool ldown, bool lclick, bool lrelease,
                      TitanUI* ui, float ox, float oy) override
    {
        if (!visible || !enabled) return false;
        return Contains(mx, my, ox, oy) && (ldown || lclick);
    }

    void Render(LightningEngine::Renderer& r, TitanFont& f, float ox, float oy) override
    {
        if (!visible) return;
        float ax = ox+x, ay = oy+y;
        const Style& s = gStyle;

        // Background
        r.SetDrawColor(s.panelBg.r, s.panelBg.g, s.panelBg.b);
        r.FillRect(ax, ay, w, h);
        r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b);
        r.DrawRect(ax, ay, w, h);

        // Title
        float curY = ay;
        if (!titleStr.empty()) {
            r.SetDrawColor(s.textBright.r, s.textBright.g, s.textBright.b);
            float tw = f.MeasureW(titleStr.c_str());
            f.DrawText(r, titleStr.c_str(), ax + (w - tw) * 0.5f, curY + 3.f);
            curY += f.GlyphH() + 6.f;
        }

        // Legend height
        float legendH = (showLegend && !series.empty()) ? (f.GlyphH() + 6.f) : 0.f;

        // Plot area insets
        float padL  = 44.f;   // Y axis labels
        float padR  = 10.f;
        float padT  = curY - ay + 4.f;
        float padB  = 22.f + legendH;  // X axis labels + legend

        float plotX = ax + padL;
        float plotY = ay + padT;
        float plotW = w - padL - padR;
        float plotH = h - padT - padB;

        if (plotW < 20.f || plotH < 20.f) return;

        // Compute Y range
        float dataMin = 0.f, dataMax = 1.f;
        bool  hasData = false;
        for (const auto& ser : series) {
            for (float v : ser.values) {
                if (!hasData) { dataMin = dataMax = v; hasData = true; }
                else { dataMin = std::min(dataMin, v); dataMax = std::max(dataMax, v); }
            }
        }
        float effMin = (yMin == 0.f && yMax == 0.f) ? dataMin : yMin;
        float effMax = (yMin == 0.f && yMax == 0.f) ? dataMax : yMax;
        if (effMax <= effMin) effMax = effMin + 1.f;

        // Y scale helper
        auto toScreenY = [&](float v) -> float {
            return plotY + plotH - (v - effMin) / (effMax - effMin) * plotH;
        };

        // Plot background
        r.SetDrawColor(18, 18, 25);
        r.FillRect(plotX, plotY, plotW, plotH);

        // Grid lines (horizontal)
        if (showGrid) {
            int gridLines = 4;
            r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b, 80);
            for (int i = 0; i <= gridLines; i++) {
                float gy = plotY + plotH * i / gridLines;
                r.FillRect(plotX, gy, plotW, 1.f);
            }
        }

        // Y axis labels
        {
            char buf[32];
            int  steps = 4;
            r.SetDrawColor(s.textDim.r, s.textDim.g, s.textDim.b);
            for (int i = 0; i <= steps; i++) {
                float v  = effMin + (effMax - effMin) * i / steps;
                float gy = toScreenY(v);
                SDL_snprintf(buf, sizeof(buf), "%.1f", v);
                float lw = f.MeasureW(buf);
                f.DrawText(r, buf, plotX - lw - 3.f, gy - f.GlyphH() * 0.5f);
            }
        }

        // Axes
        r.SetDrawColor(s.panelBorder.r + 20, s.panelBorder.g + 20, s.panelBorder.b + 20);
        r.FillRect(plotX, plotY, 1.f, plotH);        // Y axis
        r.FillRect(plotX, plotY + plotH, plotW, 1.f); // X axis

        // Find max data length across all series
        int maxLen = 0;
        for (const auto& ser : series)
            maxLen = std::max(maxLen, (int)ser.values.size());
        if (maxLen == 0) maxLen = 1;

        // X axis labels (first, last, and a few in between)
        r.SetDrawColor(s.textDim.r, s.textDim.g, s.textDim.b);
        for (int li : {0, maxLen/4, maxLen/2, 3*maxLen/4, maxLen-1}) {
            if (li < 0 || li >= maxLen) continue;
            float gx = plotX + (maxLen <= 1 ? 0.f : (float)li / (maxLen-1) * plotW);
            char buf[16];
            SDL_snprintf(buf, sizeof(buf), "%d", li);
            float lw = f.MeasureW(buf);
            f.DrawText(r, buf, gx - lw*0.5f, plotY + plotH + 4.f);
        }

        // Draw each series
        if (type == Type::Line) {
            for (const auto& ser : series) {
                if (ser.values.empty()) continue;
                r.SetDrawColor(ser.r, ser.g, ser.b);
                float prevSX = 0.f, prevSY = 0.f;
                for (int i = 0; i < (int)ser.values.size(); i++) {
                    float sx = plotX + (ser.values.size() <= 1 ? 0.f
                                : (float)i / (ser.values.size()-1) * plotW);
                    float sy = toScreenY(ser.values[i]);
                    if (i > 0) drawSegment(r, prevSX, prevSY, sx, sy, lineThick);
                    // Dot at each point
                    r.FillRect(sx - 1.5f, sy - 1.5f, 3.f, 3.f);
                    prevSX = sx; prevSY = sy;
                }
            }
        } else {  // Bar
            int   numSeries = (int)series.size();
            if (numSeries == 0) numSeries = 1;
            float groupW = (maxLen > 0) ? plotW / maxLen : plotW;
            float barW   = std::max(2.f, groupW / numSeries - 2.f);
            float zeroY  = toScreenY(std::max(effMin, 0.f));

            for (int si = 0; si < (int)series.size(); si++) {
                const auto& ser = series[si];
                r.SetDrawColor(ser.r, ser.g, ser.b, 200);
                for (int i = 0; i < (int)ser.values.size(); i++) {
                    float gx   = plotX + i * groupW + si * barW;
                    float sy   = toScreenY(ser.values[i]);
                    float barH = std::abs(zeroY - sy);
                    float by   = std::min(zeroY, sy);
                    r.FillRect(gx + 1.f, by, barW, std::max(1.f, barH));
                }
            }
        }

        // Legend
        if (showLegend && !series.empty()) {
            float lx  = ax + padL;
            float ly  = ay + h - legendH + 2.f;
            float swatchW = 12.f;
            for (const auto& ser : series) {
                r.SetDrawColor(ser.r, ser.g, ser.b);
                r.FillRect(lx, ly + (f.GlyphH() - 4.f)*0.5f, swatchW, 4.f);
                lx += swatchW + 4.f;
                r.SetDrawColor(s.textNormal.r, s.textNormal.g, s.textNormal.b);
                f.DrawText(r, ser.name.c_str(), lx, ly);
                lx += f.MeasureW(ser.name.c_str()) + 12.f;
            }
        }

        // Plot border
        r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b);
        r.DrawRect(plotX, plotY, plotW, plotH);
    }

private:
    // Draw a line segment using FillRect steps (DDA approximation).
    static void drawSegment(LightningEngine::Renderer& r,
                            float x0, float y0, float x1, float y1, float thick)
    {
        float dx = x1 - x0, dy = y1 - y0;
        int   steps = (int)std::max(std::abs(dx), std::abs(dy));
        if (steps == 0) { r.FillRect(x0, y0, thick, thick); return; }
        float sx = dx / steps, sy = dy / steps;
        float px = x0, py = y0;
        float half = thick * 0.5f;
        for (int i = 0; i <= steps; i++) {
            r.FillRect(px - half, py - half, thick, thick);
            px += sx; py += sy;
        }
    }
};

} // namespace Titan
