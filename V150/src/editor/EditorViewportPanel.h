// EditorViewportPanel.h — Viewport panel: camera, gizmos, grid, picking, navigation.
#pragma once
#include "EditorPanel.h"
#include "../include/components/SpriteRenderer.h"
#include <cmath>
#include <limits>
#include <algorithm>

class EditorViewportPanel : public EditorPanel
{
public:
    explicit EditorViewportPanel(EditorContext& ctx) : ctx(ctx) {}

    // ── State ─────────────────────────────────────────────────────────────
    Viewport2D* pViewport = nullptr;
    Panel*      pCamWidget = nullptr;
    Label*      lblFps    = nullptr;
    float       fpsTimer  = 0.f;
    int         fpsCount  = 0;
    float       curFps    = 0.f;

    float viewportCamX    = 0.f;
    float viewportCamY    = 0.f;
    float viewportZoom    = 1.f;
    float viewportGridSize = 32.f;
    bool  viewportShowGrid = true;
    bool  viewportSnapToGrid = false;

    // ── Gizmo drag ───────────────────────────────────────────────────────
    enum class GizmoAxis { None, X, Y, XY };
    GizmoAxis gizmoAxis = GizmoAxis::None;
    float gizmoDragMX0 = 0.f, gizmoDragMY0 = 0.f;
    float gizmoDragNX0 = 0.f, gizmoDragNY0 = 0.f;

    float vpAX = 0.f, vpAY = 0.f;   // viewport screen offset

    // ── EditorPanel interface ─────────────────────────────────────────────

    std::unique_ptr<Widget> Build() override
    {
        auto vp = std::make_unique<Viewport2D>(0.f, 0.f, 100.f, 100.f);
        pViewport = vp.get();
        pViewport->showBg = true;
        pViewport->showBorder = true;

        pViewport->onRender = [this](Renderer& r, float ax, float ay, float vw, float vh) {
            vpAX = ax; vpAY = ay;
            r.BeginScreenSpace();
            r.SetDrawColor(20, 20, 26);
            r.FillRect(ax, ay, vw, vh);
            r.EndScreenSpace();

            r.SetScissor(ax, ay, vw, vh);
            r.SetCameraOrigin(ax, ay);
            r.SetCameraOffset(viewportCamX, viewportCamY);
            r.SetCameraZoom(viewportZoom);
            drawGrid(r, vw, vh);
            ctx.editorLevel.Render();
            r.ClearScissor();
            r.SetCameraOrigin(0.f, 0.f);
            r.SetCameraOffset(0.f, 0.f);
            r.SetCameraZoom(1.f);
            r.BeginScreenSpace();

            // Gizmos
            if (ctx.selectedNode && !ctx.isPlaying) {
                static constexpr float kArrow = 50.f;
                static constexpr float kTip   = 5.f;
                auto wp = ctx.selectedNode->WorldPosition();
                Lightning::V2 sp = worldToScreen(wp.x, wp.y);
                float nx = sp.x, ny = sp.y;

                r.SetDrawColor(220, 220, 220, 230);
                r.FillRect(nx - 5.f, ny - 5.f, 10.f, 10.f);
                r.SetDrawColor(220, 60, 60, 230);
                r.DrawLine(nx, ny, nx + kArrow, ny, 2.f);
                r.FillCircle(nx + kArrow, ny, kTip);
                r.SetDrawColor(60, 200, 60, 230);
                r.DrawLine(nx, ny, nx, ny + kArrow, 2.f);
                r.FillCircle(nx, ny + kArrow, kTip);
                r.SetDrawColor(255, 200, 0, 120);
                r.DrawRect(nx - 14.f, ny - 14.f, 28.f, 28.f);
            }

            // Empty scene label
            if (ctx.editorLevel.NodeCount() == 0) {
                const char* lbl = ctx.pm.isOpen ? "Empty Scene \xe2\x80\x94 use + Node to add objects"
                                                : "No Project Open";
                float lw = ctx.ui.font.MeasureW(lbl);
                r.SetDrawColor(60, 60, 72);
                ctx.ui.font.DrawText(r, lbl, ax + (vw - lw) * 0.5f,
                                     ay + vh * 0.5f - ctx.ui.font.GlyphH() * 0.5f);
            }

            // Playing indicator
            if (ctx.isPlaying) {
                const char* msg = "\xe2\x97\x8f PLAYING";
                float mw = ctx.ui.font.MeasureW(msg);
                r.SetDrawColor(80, 200, 80, 200);
                ctx.ui.font.DrawText(r, msg, ax + vw - mw - 8.f, ay + 6.f);
            }

            // HUD
            char hud[128];
            SDL_snprintf(hud, sizeof(hud), "Zoom %.0f%%  |  Grid %s  |  Snap %s",
                         viewportZoom * 100.f,
                         viewportShowGrid ? "ON" : "OFF",
                         viewportSnapToGrid ? "ON" : "OFF");
            float hudW = ctx.ui.font.MeasureW(hud);
            r.SetDrawColor(18, 18, 24, 185);
            r.FillRect(ax + 10.f, ay + 8.f, hudW + 12.f, ctx.ui.font.GlyphH() + 8.f);
            r.SetDrawColor(210, 210, 220, 230);
            ctx.ui.font.DrawText(r, hud, ax + 16.f, ay + 12.f);

            std::string watermark = fs::path(ctx.currentScenePath).stem().string();
            if (watermark.empty()) watermark = "Scene2D";
            float wmW = ctx.ui.font.MeasureW(watermark.c_str());
            r.SetDrawColor(255, 255, 255, 110);
            ctx.ui.font.DrawText(r, watermark.c_str(), ax + vw - wmW - 18.f,
                                 ay + vh - ctx.ui.font.GlyphH() - 18.f);
            r.EndScreenSpace();
        };

        return vp;
    }

    std::unique_ptr<Widget> BuildCameraWidget(float widgetW, float camH)
    {
        auto panel = std::make_unique<Panel>(0.f, 0.f, widgetW, camH, "", false);
        pCamWidget = panel.get();

        const float lh  = gStyle.lineH;
        const float rX  = 90.f;
        const float gSz = 15.f;
        const float hw  = (widgetW - rX - gStyle.padding - 3.f) / 2.f;
        float ty = 4.f;

        pCamWidget->Add<Button>(widgetW - gSz - 4.f, ty, gSz, gSz, "S")->onClick = []{};
        pCamWidget->Add<Button>(rX, ty, widgetW - rX - gSz - 8.f, lh, "MAX CINEMATIC")->onClick = []{};
        ty += lh + 3.f;

        lblFps = pCamWidget->Add<Label>(rX, ty + (lh - ctx.ui.font.GlyphH()) * 0.5f, "0.0 fps");
        lblFps->SetColor(gStyle.textGreen.r, gStyle.textGreen.g, gStyle.textGreen.b);
        pCamWidget->Add<Button>(rX + hw + 3.f, ty, hw, lh, "RTRT")->onClick = []{};
        ty += lh + 3.f;

        auto* lRes = pCamWidget->Add<Label>(rX, ty + (lh - ctx.ui.font.GlyphH()) * 0.5f, "100% RES");
        lRes->SetColor(gStyle.textDim.r, gStyle.textDim.g, gStyle.textDim.b);
        pCamWidget->Add<Label>(rX + hw + 3.f, ty + (lh - ctx.ui.font.GlyphH()) * 0.5f, "1280p")
                  ->SetColor(gStyle.textDim.r, gStyle.textDim.g, gStyle.textDim.b);

        return panel;
    }

    void Update(float dt) override
    {
        processNavigation();
        if (!ctx.isPlaying && ctx.selectedNode)
            processGizmoDrag();
        processSelection();

        // FPS
        fpsTimer += dt / 1000.f;
        fpsCount++;
        if (fpsTimer >= 1.f) {
            curFps = (float)fpsCount / fpsTimer;
            fpsCount = 0; fpsTimer = 0.f;
            if (lblFps) {
                char buf[32];
                SDL_snprintf(buf, sizeof(buf), "%.1f fps", curFps);
                lblFps->SetText(buf);
            }
        }
    }

    void ClearPtrs() override
    {
        pViewport = nullptr;
        pCamWidget = nullptr;
        lblFps = nullptr;
    }

    // ── Coordinate conversion ─────────────────────────────────────────────

    bool isMouseOver(float mx, float my) const
    {
        if (!pViewport) return false;
        return mx >= pViewport->x && mx <= pViewport->x + pViewport->w &&
               my >= pViewport->y && my <= pViewport->y + pViewport->h;
    }

    Lightning::V2 screenToWorld(float sx, float sy) const
    {
        float zoom = std::max(viewportZoom, 0.001f);
        return { viewportCamX + (sx - vpAX) / zoom,
                 viewportCamY + (sy - vpAY) / zoom };
    }

    Lightning::V2 worldToScreen(float wx, float wy) const
    {
        float zoom = std::max(viewportZoom, 0.001f);
        return { vpAX + (wx - viewportCamX) * zoom,
                 vpAY + (wy - viewportCamY) * zoom };
    }

    float snapValue(float v) const
    {
        if (!viewportSnapToGrid || viewportGridSize <= 0.f) return v;
        return std::round(v / viewportGridSize) * viewportGridSize;
    }

    Lightning::V2 snapPoint(Lightning::V2 p) const
    {
        return { snapValue(p.x), snapValue(p.y) };
    }

    void focusOn(const Lightning::V2& worldPoint)
    {
        if (!pViewport) return;
        float zoom = std::max(viewportZoom, 0.001f);
        viewportCamX = worldPoint.x - (pViewport->w * 0.5f) / zoom;
        viewportCamY = worldPoint.y - (pViewport->h * 0.5f) / zoom;
        if (ctx.noteChange) ctx.noteChange("Viewport focus changed");
    }

    void frameSelection()
    {
        if (ctx.selectedNode) {
            focusOn({ ctx.selectedNode->WorldPosition().x,
                      ctx.selectedNode->WorldPosition().y });
            return;
        }
        const auto& nodes = ctx.editorLevel.GetNodes();
        if (nodes.empty()) { resetView(); return; }

        float minX = 1e9f, minY = 1e9f, maxX = -1e9f, maxY = -1e9f;
        for (const auto& n : nodes) {
            auto wp = n->WorldPosition();
            minX = std::min(minX, wp.x); minY = std::min(minY, wp.y);
            maxX = std::max(maxX, wp.x); maxY = std::max(maxY, wp.y);
        }
        focusOn({ (minX + maxX) * 0.5f, (minY + maxY) * 0.5f });
    }

    void resetView()
    {
        viewportCamX = 0.f; viewportCamY = 0.f; viewportZoom = 1.f;
    }

    void syncToolbarState(Toolbar* pToolbar)
    {
        if (!pToolbar || pToolbar->items.size() < 8) return;
        pToolbar->items[4].active = viewportShowGrid;
        pToolbar->items[5].active = viewportSnapToGrid;
    }

    bool isCursorOverGizmo(float mx, float my) const
    {
        if (!ctx.selectedNode || ctx.isPlaying) return false;
        static constexpr float kArrow = 50.f;
        static constexpr float kHitR = 9.f;
        static constexpr float kCenterR = 7.f;

        Lightning::V2 wp = ctx.selectedNode->WorldPosition();
        Lightning::V2 sp = worldToScreen(wp.x, wp.y);
        auto dist = [](float ax, float ay, float bx, float by) {
            float dx = ax - bx, dy = ay - by;
            return std::sqrt(dx * dx + dy * dy);
        };
        return dist(mx, my, sp.x, sp.y) < kCenterR ||
               dist(mx, my, sp.x + kArrow, sp.y) < kHitR ||
               dist(mx, my, sp.x, sp.y + kArrow) < kHitR;
    }

    // ── Cache save/load helpers ───────────────────────────────────────────

    void SaveCache(IniFile& ini) const
    {
        ini.Set("Viewport", "CamX", std::to_string(viewportCamX));
        ini.Set("Viewport", "CamY", std::to_string(viewportCamY));
        ini.Set("Viewport", "Zoom", std::to_string(viewportZoom));
        ini.Set("Viewport", "ShowGrid", viewportShowGrid ? "true" : "false");
        ini.Set("Viewport", "SnapToGrid", viewportSnapToGrid ? "true" : "false");
    }

    void LoadCache(IniFile& ini)
    {
        auto readFloat = [&](const char* sec, const char* key, float defVal) {
            try { return std::stof(ini.Get(sec, key, std::to_string(defVal))); }
            catch (...) { return defVal; }
        };
        viewportCamX = readFloat("Viewport", "CamX", viewportCamX);
        viewportCamY = readFloat("Viewport", "CamY", viewportCamY);
        viewportZoom = std::clamp(readFloat("Viewport", "Zoom", viewportZoom), 0.2f, 8.0f);
        viewportShowGrid = ini.GetBool("Viewport", "ShowGrid", viewportShowGrid);
        viewportSnapToGrid = ini.GetBool("Viewport", "SnapToGrid", viewportSnapToGrid);
    }

private:
    EditorContext& ctx;

    // Panning state
    bool  viewportPanning = false;
    float viewportPanStartMX = 0.f, viewportPanStartMY = 0.f;
    float viewportPanCamX0 = 0.f, viewportPanCamY0 = 0.f;

    void drawGrid(Renderer& r, float vw, float vh)
    {
        if (!viewportShowGrid || viewportGridSize <= 0.f) return;
        float zoom = std::max(viewportZoom, 0.001f);
        float step = viewportGridSize;
        while (step * zoom < 14.f) step *= 2.f;

        float left = viewportCamX, top = viewportCamY;
        float right = viewportCamX + vw / zoom;
        float bottom = viewportCamY + vh / zoom;
        float thickness = 1.f / zoom;

        float startX = std::floor(left / step) * step;
        for (float x = startX; x <= right + step; x += step) {
            bool ax = std::fabs(x) < 0.001f;
            r.SetDrawColor(ax ? 120 : 58, ax ? 120 : 58, ax ? 132 : 70, ax ? 180 : 90);
            r.DrawLine(x, top, x, bottom, thickness);
        }
        float startY = std::floor(top / step) * step;
        for (float y = startY; y <= bottom + step; y += step) {
            bool ax = std::fabs(y) < 0.001f;
            r.SetDrawColor(ax ? 120 : 58, ax ? 120 : 58, ax ? 132 : 70, ax ? 180 : 90);
            r.DrawLine(left, y, right, y, thickness);
        }
    }

    void pickNodeRecursive(Node* node, float wX, float wY, float rW,
                           Node*& bestNode, float& bestScore)
    {
        if (!node || !node->active) return;
        Lightning::V2 wp = node->WorldPosition();
        bool hit = false;
        float score = std::numeric_limits<float>::max();

        if (auto* sprite = node->GetComponent<SpriteRendererComponent>()) {
            float w = sprite->width > 0.f ? sprite->width :
                      (sprite->texture ? (float)sprite->texture->GetWidth() : 32.f);
            float h = sprite->height > 0.f ? sprite->height :
                      (sprite->texture ? (float)sprite->texture->GetHeight() : 32.f);
            w *= node->transform.Scale.x;
            h *= node->transform.Scale.y;
            float x = wp.x - w * sprite->pivot.x;
            float y = wp.y - h * sprite->pivot.y;
            if (wX >= x && wX <= x + w && wY >= y && wY <= y + h) {
                hit = true; score = w * h;
            }
        }
        if (!hit) {
            float dx = wX - wp.x, dy = wY - wp.y;
            float d2 = dx * dx + dy * dy;
            if (d2 <= rW * rW) { hit = true; score = d2; }
        }
        if (hit && score <= bestScore) { bestNode = node; bestScore = score; }
        for (const auto& child : node->GetChildren())
            pickNodeRecursive(child.get(), wX, wY, rW, bestNode, bestScore);
    }

    Node* pickNode(float mx, float my)
    {
        Lightning::V2 world = screenToWorld(mx, my);
        float rW = 12.f / std::max(viewportZoom, 0.001f);
        Node* best = nullptr;
        float bestScore = std::numeric_limits<float>::max();
        const auto& nodes = ctx.editorLevel.GetNodes();
        for (auto it = nodes.rbegin(); it != nodes.rend(); ++it)
            pickNodeRecursive(it->get(), world.x, world.y, rW, best, bestScore);
        return best;
    }

    void processNavigation()
    {
        if (!pViewport || ctx.isPlaying) return;
        float mx = ctx.inputManager.GetMouseX();
        float my = ctx.inputManager.GetMouseY();
        bool over = isMouseOver(mx, my);

        if (over) {
            bool changed = false;
            float scroll = ctx.inputManager.GetScrollWheelY();
            if (scroll != 0.f) {
                Lightning::V2 before = screenToWorld(mx, my);
                float zf = std::pow(1.12f, scroll);
                viewportZoom = std::clamp(viewportZoom * zf, 0.2f, 8.0f);
                viewportCamX = before.x - (mx - vpAX) / viewportZoom;
                viewportCamY = before.y - (my - vpAY) / viewportZoom;
                changed = true;
            }
            if (ctx.inputManager.IsKeyPressed(SDL_SCANCODE_G)) { viewportShowGrid = !viewportShowGrid; changed = true; }
            if (ctx.inputManager.IsKeyPressed(SDL_SCANCODE_S)) { viewportSnapToGrid = !viewportSnapToGrid; changed = true; }
            if (ctx.inputManager.IsKeyPressed(SDL_SCANCODE_F)) frameSelection();
            if (ctx.inputManager.IsKeyPressed(SDL_SCANCODE_0)) resetView();
            if (changed && ctx.noteChange) ctx.noteChange("Viewport settings changed");
        }

        if (ctx.inputManager.IsMousePressed(2) && over) {
            viewportPanning = true;
            viewportPanStartMX = mx; viewportPanStartMY = my;
            viewportPanCamX0 = viewportCamX; viewportPanCamY0 = viewportCamY;
        }
        if (viewportPanning) {
            if (ctx.inputManager.IsMouseDown(2)) {
                float zoom = std::max(viewportZoom, 0.001f);
                viewportCamX = viewportPanCamX0 - (mx - viewportPanStartMX) / zoom;
                viewportCamY = viewportPanCamY0 - (my - viewportPanStartMY) / zoom;
            } else {
                viewportPanning = false;
            }
        }
    }

    void processSelection()
    {
        if (!pViewport || ctx.isPlaying || viewportPanning || gizmoAxis != GizmoAxis::None) return;
        if (!ctx.inputManager.IsMousePressed(1)) return;
        float mx = ctx.inputManager.GetMouseX();
        float my = ctx.inputManager.GetMouseY();
        if (!isMouseOver(mx, my) || isCursorOverGizmo(mx, my)) return;

        Node* picked = pickNode(mx, my);
        if (picked != ctx.selectedNode) {
            ctx.selectedNode = picked;
            if (ctx.onHierarchyChanged) ctx.onHierarchyChanged();
            if (ctx.onInspectorRefresh) ctx.onInspectorRefresh();
        } else if (!picked) {
            if (ctx.onInspectorRefresh) ctx.onInspectorRefresh();
        }
    }

    void processGizmoDrag()
    {
        if (!ctx.selectedNode) return;
        float mx = ctx.inputManager.GetMouseX();
        float my = ctx.inputManager.GetMouseY();

        static constexpr float kArrow = 50.f;
        static constexpr float kHitR = 9.f;
        static constexpr float kCenterR = 7.f;

        auto wp = ctx.selectedNode->WorldPosition();
        Lightning::V2 sp = worldToScreen(wp.x, wp.y);
        float nx = sp.x, ny = sp.y;

        auto dist = [](float ax, float ay, float bx, float by) {
            float dx = ax - bx, dy = ay - by;
            return std::sqrt(dx * dx + dy * dy);
        };

        if (ctx.inputManager.IsMousePressed(1) && gizmoAxis == GizmoAxis::None) {
            bool nearX  = dist(mx, my, nx + kArrow, ny) < kHitR;
            bool nearY  = dist(mx, my, nx, ny + kArrow) < kHitR;
            bool nearXY = dist(mx, my, nx, ny) < kCenterR;
            if (nearX || nearY || nearXY) {
                gizmoAxis = nearXY ? GizmoAxis::XY : nearX ? GizmoAxis::X : GizmoAxis::Y;
                gizmoDragMX0 = mx; gizmoDragMY0 = my;
                gizmoDragNX0 = ctx.selectedNode->transform.Position.x;
                gizmoDragNY0 = ctx.selectedNode->transform.Position.y;
            }
        }

        if (gizmoAxis != GizmoAxis::None && ctx.inputManager.IsMouseDown(1)) {
            float zoom = std::max(viewportZoom, 0.001f);
            float dx = (mx - gizmoDragMX0) / zoom;
            float dy = (my - gizmoDragMY0) / zoom;
            if (gizmoAxis == GizmoAxis::X || gizmoAxis == GizmoAxis::XY)
                ctx.selectedNode->transform.Position.x = snapValue(gizmoDragNX0 + dx);
            if (gizmoAxis == GizmoAxis::Y || gizmoAxis == GizmoAxis::XY)
                ctx.selectedNode->transform.Position.y = snapValue(gizmoDragNY0 + dy);
            ctx.selectedNode->MarkTransformDirty();
            if (ctx.onInspectorRefresh) ctx.onInspectorRefresh();
        }

        if (gizmoAxis != GizmoAxis::None && !ctx.inputManager.IsMouseDown(1)) {
            GizmoAxis axis = gizmoAxis;
            float nx0 = gizmoDragNX0, ny0 = gizmoDragNY0;
            float nxF = ctx.selectedNode->transform.Position.x;
            float nyF = ctx.selectedNode->transform.Position.y;
            Node* node = ctx.selectedNode;

            ctx.undoStack.Push(
                [node, axis, nxF, nyF](){
                    if (axis == GizmoAxis::X || axis == GizmoAxis::XY) node->transform.Position.x = nxF;
                    if (axis == GizmoAxis::Y || axis == GizmoAxis::XY) node->transform.Position.y = nyF;
                    node->MarkTransformDirty();
                },
                [this, node, axis, nx0, ny0](){
                    if (axis == GizmoAxis::X || axis == GizmoAxis::XY) node->transform.Position.x = nx0;
                    if (axis == GizmoAxis::Y || axis == GizmoAxis::XY) node->transform.Position.y = ny0;
                    node->MarkTransformDirty();
                    if (ctx.onInspectorRefresh) ctx.onInspectorRefresh();
                },
                "Move Gizmo"
            );
            gizmoAxis = GizmoAxis::None;
            if (ctx.onInspectorRefresh) ctx.onInspectorRefresh();
        }
    }
};
