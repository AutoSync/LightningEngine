// TitanFont.h — Titan GUI TrueType font renderer (stb_truetype).
// Bakes a TTF file into a GPU glyph atlas at a given pixel size.
// The STB implementation is compiled in src/core/TitanFont.cpp.
#pragma once
#include <vector>
#include <cstring>
#include <algorithm>
#include <SDL3/SDL.h>
#include "../Renderer.h"
#include "../Texture.h"
#include "../../third_party/stb/stb_truetype.h"

namespace Titan {

class TitanFont {
public:
    LightningEngine::Texture atlas;
    int   pixelSize  = 13;
    int   atlasW     = 512;
    int   atlasH     = 256;
    float baseline   = 0.f;    // pixels from top-of-line to baseline
    float lineHeight = 0.f;    // total line height in pixels

    stbtt_bakedchar glyphs[96]; // ASCII 32..127

    // Load TTF at the given pixel size and build GPU atlas.
    // Tries atlasH 256 then 512 if 256 is too small.
    bool Build(LightningEngine::Renderer& r, const char* path, int size = 13)
    {
        pixelSize = size;

        size_t dataBytes = 0;
        void*  rawData   = SDL_LoadFile(path, &dataBytes);
        if (!rawData) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "[TitanFont] Cannot load font: %s", path);
            return false;
        }

        // Compute font metrics (baseline, line height)
        stbtt_fontinfo info;
        stbtt_InitFont(&info,
            reinterpret_cast<const unsigned char*>(rawData),
            stbtt_GetFontOffsetForIndex(
                reinterpret_cast<const unsigned char*>(rawData), 0));
        int asc = 0, desc = 0, lineGap = 0;
        stbtt_GetFontVMetrics(&info, &asc, &desc, &lineGap);
        float sc = stbtt_ScaleForPixelHeight(&info, static_cast<float>(pixelSize));
        baseline   = asc  * sc;
        lineHeight = (asc - desc + lineGap) * sc;

        // Try to bake at progressively larger atlas heights
        for (int tryH : { 256, 512 }) {
            atlasH = tryH;
            std::vector<unsigned char> bitmap(atlasW * tryH, 0);
            int rows = stbtt_BakeFontBitmap(
                reinterpret_cast<const unsigned char*>(rawData), 0,
                static_cast<float>(pixelSize),
                bitmap.data(), atlasW, tryH,
                32, 96, glyphs);

            if (rows > 0) {
                SDL_free(rawData);
                // Convert alpha-only → RGBA (white with alpha)
                std::vector<unsigned char> rgba(atlasW * tryH * 4);
                for (int i = 0; i < atlasW * tryH; i++) {
                    rgba[i*4+0] = 255;
                    rgba[i*4+1] = 255;
                    rgba[i*4+2] = 255;
                    rgba[i*4+3] = bitmap[i];
                }
                atlas = r.LoadTextureFromPixels(rgba.data(), atlasW, tryH, false);
                return atlas.IsValid();
            }
        }

        SDL_free(rawData);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "[TitanFont] Glyph atlas too small for size %d", size);
        return false;
    }

    void Release() { atlas.Release(); }

    bool IsValid() const { return atlas.IsValid(); }

    // Measure text width in pixels (no scale — fixed at baked size).
    float MeasureW(const char* text) const
    {
        if (!text || !atlas.IsValid()) return 0.f;
        float cx = 0.f, cy = 0.f;
        while (*text) {
            char c = *text++;
            if (c < 32 || c > 127) continue;
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(const_cast<stbtt_bakedchar*>(glyphs),
                               atlasW, atlasH,
                               static_cast<int>(c - 32),
                               &cx, &cy, &q, 1);
        }
        return cx;
    }

    // Height of a capital letter (useful for vertical centring).
    float GlyphH() const { return baseline; }

    // Draw text at (x, y) where y = top of the line.
    // SetDrawColor before calling to tint the font.
    void DrawText(LightningEngine::Renderer& r,
                  const char* text, float x, float y) const
    {
        if (!text || !atlas.IsValid()) return;
        float cx     = x;
        float base_y = y + baseline; // convert top → baseline

        while (*text) {
            char c = *text++;
            if (c == '\n') { cx = x; base_y += lineHeight; continue; }
            if (c < 32 || c > 127) c = '?';

            stbtt_aligned_quad q;
            float tmpY = base_y;
            stbtt_GetBakedQuad(const_cast<stbtt_bakedchar*>(glyphs),
                               atlasW, atlasH,
                               static_cast<int>(c - 32),
                               &cx, &tmpY, &q, 1);

            r.DrawTextureRegion(
                const_cast<LightningEngine::Texture&>(atlas),
                q.x0, q.y0,
                q.x1 - q.x0, q.y1 - q.y0,
                q.s0, q.t0, q.s1, q.t1);
        }
    }
};

} // namespace Titan
