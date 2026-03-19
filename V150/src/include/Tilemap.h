// Tilemap.h — Grid-based tile renderer using a texture atlas.
//
// Usage:
//   Texture atlas = renderer.LoadTexture("assets/textures/tiles.png");
//
//   Tilemap map;
//   map.atlas  = &atlas;
//   map.tileW  = 16;
//   map.tileH  = 16;
//   map.atlasColCount = 8;   // how many tiles wide the atlas is
//   map.Resize(20, 15);      // 20 columns, 15 rows
//   map.SetTile(0, 0, 5);    // tile at (col=0, row=0) = atlas index 5
//
//   // In Render():
//   map.Draw(renderer, worldX, worldY);
//
// TilemapComponent: attach to a Node for automatic draw via component system.
//   node->AddComponent<TilemapComponent>()->map = myMap;
#pragma once
#include <vector>
#include <SDL3/SDL.h>
#include "Renderer.h"
#include "Texture.h"
#include "Component.h"

namespace LightningEngine {

class Tilemap {
public:
    Texture* atlas         = nullptr;  // not owned
    int      tileW         = 16;       // tile width  in pixels
    int      tileH         = 16;       // tile height in pixels
    int      atlasColCount = 1;        // tiles per row in the atlas

    // Row-major grid: cells[row][col] = tile index, -1 = empty.
    std::vector<std::vector<int>> cells;

    // Resize the map. Fills new cells with -1.
    void Resize(int cols, int rows)
    {
        cells.assign(rows, std::vector<int>(cols, -1));
    }

    int  ColCount() const { return cells.empty()    ? 0 : (int)cells[0].size(); }
    int  RowCount() const { return (int)cells.size(); }

    void SetTile(int col, int row, int tileIdx)
    {
        if (row >= 0 && row < RowCount() &&
            col >= 0 && col < ColCount())
            cells[row][col] = tileIdx;
    }

    int GetTile(int col, int row) const
    {
        if (row < 0 || row >= RowCount() || col < 0 || col >= ColCount())
            return -1;
        return cells[row][col];
    }

    // Draw all tiles to the renderer.
    // worldX/worldY: top-left world position of the tilemap.
    void Draw(Renderer& r, float worldX = 0.f, float worldY = 0.f) const
    {
        if (!atlas || !atlas->IsValid() || atlasColCount <= 0) return;

        int atlasW = atlas->GetWidth();
        int atlasH = atlas->GetHeight();
        float invW = 1.f / (float)atlasW;
        float invH = 1.f / (float)atlasH;
        float fTileW = (float)tileW;
        float fTileH = (float)tileH;

        for (int row = 0; row < RowCount(); ++row) {
            for (int col = 0; col < ColCount(); ++col) {
                int idx = cells[row][col];
                if (idx < 0) continue;

                float sx = worldX + col * fTileW;
                float sy = worldY + row * fTileH;

                int tileCol = idx % atlasColCount;
                int tileRow = idx / atlasColCount;

                float u0 = tileCol * fTileW * invW;
                float v0 = tileRow * fTileH * invH;
                float u1 = u0 + fTileW * invW;
                float v1 = v0 + fTileH * invH;

                r.DrawTextureRegion(*atlas, sx, sy, fTileW, fTileH, u0, v0, u1, v1);
            }
        }
    }

    // Draw a single tile by index at screen position (sx, sy).
    void DrawTile(Renderer& r, float sx, float sy, int idx) const
    {
        if (!atlas || !atlas->IsValid() || idx < 0 || atlasColCount <= 0) return;

        float invW  = 1.f / (float)atlas->GetWidth();
        float invH  = 1.f / (float)atlas->GetHeight();
        float fTileW = (float)tileW;
        float fTileH = (float)tileH;

        int tileCol = idx % atlasColCount;
        int tileRow = idx / atlasColCount;
        float u0 = tileCol * fTileW * invW;
        float v0 = tileRow * fTileH * invH;
        r.DrawTextureRegion(*atlas, sx, sy, fTileW, fTileH,
                            u0, v0, u0 + fTileW * invW, v0 + fTileH * invH);
    }
};

// ── TilemapComponent ─────────────────────────────────────────────────────────

class TilemapComponent : public Component {
public:
    Tilemap map;

    void Render() override
    {
        if (!owner || !owner->renderer) return;
        auto wp = owner->WorldPosition();
        map.Draw(*owner->renderer, wp.x, wp.y);
    }
};

} // namespace LightningEngine
