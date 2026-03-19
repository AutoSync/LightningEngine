// DockSpace.h — Docking layout manager.
//
// Architecture:
//   DockNode binary tree — each node is either:
//     SPLIT : two children separated by a draggable splitter
//     LEAF  : holds 1..N panels (tabbed); DockNode OWNS each panel via unique_ptr
//
//   DockSpace is a TitanUI root widget covering the main content area.
//   It renders tab bars, splitter handles, drop-zone arrows, and the
//   content of every active panel via panel->Render() with absolute coords.
//
//   Floating (undocked) panels are transferred to TitanUI roots via
//   TitanUI::AdoptRoot / TitanUI::ReleaseRoot.
//
// Usage (EditorApp):
//   auto* ds = ui.AddRoot<DockSpace>(0.f, kTopH, kW, kMainH);
//   auto [left, rest]  = ds->Root()->Split(true,  kLeftW / kW);
//   auto [vp,  right]  = rest->Split(true,  1.f - kHierW / rest->w);
//   auto [cam, browse] = left->Split(false, kCamH  / kMainH);
//   auto [hier, insp]  = right->Split(false, kHierSplitH / kMainH);
//   cam->Dock(std::move(camPanel),    "Camera");
//   browse->Dock(std::move(cbPanel),  "Content Browser");
//   vp->Dock(std::move(vpPanel),      "Viewport");
//   hier->Dock(std::move(hierPanel),  "Hierarchy View");
//   insp->Dock(std::move(inspPanel),  "Inspector");
#pragma once
#include <vector>
#include <string>
#include <memory>
#include <utility>
#include <cmath>
#include "../Widget.h"
#include "../TitanStyle.h"
#include "../TitanFont.h"

namespace Titan {

class DockSpace;

// ─────────────────────────────────────────────────────────────────────────────
// DockNode
// ─────────────────────────────────────────────────────────────────────────────
struct DockNode {
    enum class Type { Split, Leaf };

    Type       type   = Type::Leaf;
    float      x=0, y=0, w=0, h=0;   // absolute screen geometry
    DockNode*  parent = nullptr;
    DockSpace* space  = nullptr;

    // ── SPLIT state ──────────────────────────────────────────────────────────
    bool   splitV = true;             // true = left|right,  false = top|bottom
    float  ratio  = 0.5f;            // fraction of total size for childA
    static constexpr float kHandleW = 5.f;
    std::unique_ptr<DockNode> childA, childB;
    bool   hoverHandle = false;
    bool   draggHandle = false;
    float  dragOrigin  = 0.f;
    float  dragRatio   = 0.5f;

    // ── LEAF state ───────────────────────────────────────────────────────────
    struct Entry {
        std::string              title;
        std::unique_ptr<Widget>  widget;   // OWNED
        std::function<void(float w, float h)> onLayout;  // called after geometry update
    };
    std::vector<Entry>  panels;
    int   activeIdx    = 0;
    int   hoverTab     = -1;
    int   hoverTabDot  = -1;  // index of per-tab close dot being hovered
    bool  minimized    = false;
    bool  hoverMinBtn  = false;
    bool  hoverCloseBtn= false;
    mutable std::vector<float> tabWidths; // cached by Render, used by ProcessInput

    static constexpr float kTabH       = 22.f;
    static constexpr float kTabPX      = 10.f;
    static constexpr float kDragHandleW = 14.f; // ⋮ grip on left of tab bar
    static constexpr float kBtnW        = 14.f; // width of — and ● buttons
    static constexpr float kTabDotR     =  3.5f;// radius of per-tab ●/○ close dot

    // ─────────────────────────────────────────────────────────────────────────
    // Tree API
    // ─────────────────────────────────────────────────────────────────────────

    // Split this LEAF into two new LEAF children.
    // Existing panels move to childA. Returns {childA, childB}.
    std::pair<DockNode*, DockNode*> Split(bool vertical, float splitRatio = 0.5f)
    {
        if (type != Type::Leaf) return {nullptr, nullptr};

        type   = Type::Split;
        splitV = vertical;
        ratio  = splitRatio;

        auto a = std::make_unique<DockNode>();
        auto b = std::make_unique<DockNode>();
        a->parent = b->parent = this;
        a->space  = b->space  = space;
        a->type   = Type::Leaf;
        b->type   = Type::Leaf;

        // Transfer existing panels to childA
        a->panels    = std::move(panels);
        a->activeIdx = activeIdx;
        panels.clear();
        activeIdx = 0;

        DockNode* pa = a.get();
        DockNode* pb = b.get();
        childA = std::move(a);
        childB = std::move(b);

        computeChildGeometry();
        return {pa, pb};
    }

    // Add an owned panel to this LEAF.
    Widget* Dock(std::unique_ptr<Widget> w, const char* title)
    {
        if (type != Type::Leaf || !w) return nullptr;
        Widget* ptr = w.get();
        panels.push_back({ title ? title : "", std::move(w) });
        activeIdx = (int)panels.size() - 1;
        applyPanelGeometry();
        return ptr;
    }

    // Remove a panel from this LEAF (caller takes ownership).
    std::unique_ptr<Widget> Undock(Widget* target)
    {
        for (int i = 0; i < (int)panels.size(); i++) {
            if (panels[i].widget.get() == target) {
                auto owned = std::move(panels[i].widget);
                panels.erase(panels.begin() + i);
                if (activeIdx >= (int)panels.size())
                    activeIdx = (int)panels.size() - 1;
                if (activeIdx < 0) activeIdx = 0;
                applyPanelGeometry();
                return owned;
            }
        }
        return nullptr;
    }

    // Remove empty leaf by collapsing parent split.
    // Call after Undock when panels is empty and parent is SPLIT.
    void CollapseIfEmpty()
    {
        if (type != Type::Leaf || !panels.empty() || !parent) return;
        if (parent->type != Type::Split) return;

        // Find sibling
        bool isA = (parent->childA.get() == this);
        std::unique_ptr<DockNode>& keepRef = isA ? parent->childB : parent->childA;

        // Steal geometry and content from sibling into parent
        auto sibling = std::move(keepRef);
        parent->type     = sibling->type;
        parent->splitV   = sibling->splitV;
        parent->ratio    = sibling->ratio;
        parent->panels   = std::move(sibling->panels);
        parent->activeIdx= sibling->activeIdx;
        parent->childA   = std::move(sibling->childA);
        parent->childB   = std::move(sibling->childB);

        // Fix parent pointers in adopted children
        if (parent->childA) { parent->childA->parent = parent; parent->childA->space = parent->space; }
        if (parent->childB) { parent->childB->parent = parent; parent->childB->space = parent->space; }
        for (auto& e : parent->panels) if (e.widget) { e.widget->x = parent->x; e.widget->y = parent->y; }

        parent->applyPanelGeometry();
        // 'this' is now deleted via the unique_ptr that was cleared above
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Geometry
    // ─────────────────────────────────────────────────────────────────────────

    void SetGeometry(float X, float Y, float W, float H)
    {
        x = X; y = Y; w = W; h = H;
        if (type == Type::Split) computeChildGeometry();
        else                     applyPanelGeometry();
    }

    void computeChildGeometry()
    {
        if (!childA || !childB) return;
        if (splitV) {
            float aW   = ratio * w - kHandleW * 0.5f;
            float bOff = ratio * w + kHandleW * 0.5f;
            float bW   = w - bOff;
            childA->SetGeometry(x,        y, std::max(aW, 1.f), h);
            childB->SetGeometry(x + bOff, y, std::max(bW, 1.f), h);
        } else {
            float aH   = ratio * h - kHandleW * 0.5f;
            float bOff = ratio * h + kHandleW * 0.5f;
            float bH   = h - bOff;
            childA->SetGeometry(x, y,        w, std::max(aH, 1.f));
            childB->SetGeometry(x, y + bOff, w, std::max(bH, 1.f));
        }
    }

    void applyPanelGeometry()
    {
        if (type != Type::Leaf) return;
        float contentY = y + kTabH;
        float contentH = h - kTabH;
        for (int i = 0; i < (int)panels.size(); i++) {
            if (!panels[i].widget) continue;
            panels[i].widget->x       = x;
            panels[i].widget->y       = contentY;
            panels[i].widget->w       = w;
            panels[i].widget->h       = contentH;
            // Hide all panels when minimized; otherwise only the active one is visible
            panels[i].widget->visible = !minimized && (i == activeIdx);
            if (!minimized && panels[i].onLayout) panels[i].onLayout(w, contentH);
        }
    }

    // Register a layout callback that fires whenever this leaf's geometry changes.
    void SetLayout(Widget* w, std::function<void(float,float)> cb)
    {
        for (auto& e : panels)
            if (e.widget.get() == w) { e.onLayout = std::move(cb); return; }
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Search
    // ─────────────────────────────────────────────────────────────────────────

    // Find the deepest LEAF node whose geometry contains (mx, my).
    DockNode* FindLeafAt(float mx, float my)
    {
        if (mx < x || mx >= x+w || my < y || my >= y+h) return nullptr;
        if (type == Type::Leaf) return this;
        DockNode* r = childA ? childA->FindLeafAt(mx, my) : nullptr;
        if (!r && childB) r = childB->FindLeafAt(mx, my);
        return r;
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// DockSpace
// ─────────────────────────────────────────────────────────────────────────────
class DockSpace : public Widget {
public:
    std::unique_ptr<DockNode> root;

    // Drop zone IDs
    enum DropZone { DZ_None=0, DZ_Left=1, DZ_Right=2, DZ_Top=3, DZ_Bottom=4, DZ_Tab=5 };

    // Drag state (public so Panel/TitanUI can poke it)
    Widget*     dragWidget  = nullptr;
    std::string dragTitle;
    float       dragX       = 0, dragY = 0;  // current floating position
    float       dragOffX    = 0, dragOffY = 0;
    bool        isDragging  = false;

    DockSpace() = default;
    DockSpace(float x, float y, float w, float h)
    { this->x=x; this->y=y; this->w=w; this->h=h; initRoot(); }

    void initRoot()
    {
        root = std::make_unique<DockNode>();
        root->space = this;
        root->SetGeometry(x, y, w, h);
    }

    // Resize (call when window is resized)
    void Resize(float X, float Y, float W, float H)
    {
        this->x = X; this->y = Y; this->w = W; this->h = H;
        if (root) root->SetGeometry(X, Y, W, H);
    }

    DockNode* Root() { return root.get(); }

    // ── Drag API ─────────────────────────────────────────────────────────────

    void BeginDrag(Widget* w, const char* title, float mx, float my, float offX, float offY)
    {
        dragWidget = w;
        dragTitle  = title ? title : "";
        dragX      = mx - offX;
        dragY      = my - offY;
        dragOffX   = offX;
        dragOffY   = offY;
        isDragging = true;
        if (w) BringToFront(w);
    }

    void EndDrag() { isDragging = false; dragWidget = nullptr; }

    // ── ProcessInput ─────────────────────────────────────────────────────────
    bool ProcessInput(float mx, float my, bool ldown, bool lclick, bool lrelease,
                      TitanUI* ui, float ox, float oy) override
    {
        if (!visible || !enabled || !root) return false;

        // Update floating drag panel position
        if (isDragging && dragWidget) {
            dragWidget->x = mx - dragOffX;
            dragWidget->y = my - dragOffY;

            // Compute drop target
            DockNode* leaf = root->FindLeafAt(mx, my);
            dropTarget = leaf;
            dropZone   = leaf ? hitDropZone(leaf, mx, my) : DZ_None;

            if (lrelease) {
                if (dropTarget && dropZone != DZ_None)
                    performDock(dragWidget, dragTitle.c_str(), dropTarget, dropZone, ui);
                EndDrag();
                return true;
            }
            return true;
        } else {
            dropTarget = nullptr;
            dropZone   = DZ_None;
        }

        // Splitter handle dragging
        bool consumed = processNodeInput(root.get(), mx, my, ldown, lclick, lrelease, ui);
        if (consumed) return true;

        // Tab bar interaction (switch, drag to undock)
        consumed = processLeafTabs(root.get(), mx, my, ldown, lclick, lrelease, ui);
        if (consumed) return true;

        // Forward input to active panels
        consumed = forwardToLeaves(root.get(), mx, my, ldown, lclick, lrelease, ui);
        return consumed;
    }

    // ── Render ───────────────────────────────────────────────────────────────
    void Render(LightningEngine::Renderer& r, TitanFont& f, float ox, float oy) override
    {
        if (!visible || !root) return;
        renderNode(root.get(), r, f);

        // Drop zone overlay during drag
        if (isDragging && dropTarget) {
            renderDropZones(dropTarget, r, f);
        }

        // Floating drag ghost (semi-transparent panel outline following mouse)
        if (isDragging && dragWidget) {
            r.SetDrawColor(40, 80, 180, 100);
            r.FillRect(dragWidget->x, dragWidget->y, dragWidget->w, dragWidget->h);
            r.SetDrawColor(80, 140, 255, 200);
            r.DrawRect(dragWidget->x, dragWidget->y, dragWidget->w, dragWidget->h);
            // Title label
            float fh = f.GlyphH();
            r.SetDrawColor(180, 210, 255, 220);
            f.DrawText(r, dragTitle.c_str(),
                       dragWidget->x + 8.f,
                       dragWidget->y + (gStyle.titleH - fh) * 0.5f);
        }
    }

private:
    DockNode* dropTarget = nullptr;
    DropZone  dropZone   = DZ_None;

    // ── Helpers: BringToFront a floating panel ───────────────────────────────
    // (Widget* is a TitanUI root; we call zOrder on it directly)
    static void BringToFront(Widget* w) { if (w) w->zOrder = 9999; }

    // ── Tree rendering ───────────────────────────────────────────────────────
    void renderNode(DockNode* n, LightningEngine::Renderer& r, TitanFont& f)
    {
        if (!n) return;
        if (n->type == DockNode::Type::Split) {
            renderNode(n->childA.get(), r, f);
            renderNode(n->childB.get(), r, f);
            renderHandle(n, r);
        } else {
            renderLeaf(n, r, f);
        }
    }

    void renderHandle(DockNode* n, LightningEngine::Renderer& r)
    {
        const Style& s = gStyle;
        float hx, hy, hw, hh;
        if (n->splitV) {
            float mid = n->x + n->ratio * n->w;
            hx = mid - DockNode::kHandleW * 0.5f; hy = n->y;
            hw = DockNode::kHandleW;               hh = n->h;
        } else {
            float mid = n->y + n->ratio * n->h;
            hx = n->x;                              hy = mid - DockNode::kHandleW * 0.5f;
            hw = n->w;                              hh = DockNode::kHandleW;
        }
        Color c = (n->hoverHandle || n->draggHandle) ? s.highlight : s.panelBorder;
        r.SetDrawColor(c.r, c.g, c.b);
        r.FillRect(hx, hy, hw, hh);
    }

    void renderLeaf(DockNode* n, LightningEngine::Renderer& r, TitanFont& f)
    {
        const Style& s = gStyle;

        // Empty leaf placeholder
        if (n->panels.empty()) {
            r.SetDrawColor(s.panelBg.r, s.panelBg.g, s.panelBg.b);
            r.FillRect(n->x, n->y, n->w, n->h);
            r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b);
            r.DrawRect(n->x, n->y, n->w, n->h);
            return;
        }

        // ── Measurements ──────────────────────────────────────────────────────
        const float kBtnAreaW = DockNode::kBtnW * 2.f + 4.f;   // — + gap + ●
        const float kTabStartX = n->x + DockNode::kDragHandleW;
        const float kTabEndX   = n->x + n->w - kBtnAreaW;

        // ── Tab bar background ────────────────────────────────────────────────
        r.SetDrawColor(s.panelHeader.r, s.panelHeader.g, s.panelHeader.b);
        r.FillRect(n->x, n->y, n->w, DockNode::kTabH);

        // ── ⋮ Drag handle (left) ─────────────────────────────────────────────
        {
            float hx = n->x + DockNode::kDragHandleW * 0.5f;
            float hy = n->y + DockNode::kTabH * 0.5f - 3.5f;
            r.SetDrawColor(s.textDim.r, s.textDim.g, s.textDim.b);
            for (int d = 0; d < 3; d++)
                r.FillRect(hx - 1.5f, hy + (float)d * 3.5f, 3.f, 3.f);
        }

        // ── Tabs ──────────────────────────────────────────────────────────────
        bool multiTab = (int)n->panels.size() > 1;
        n->tabWidths.resize(n->panels.size());
        float tx = kTabStartX;

        for (int i = 0; i < (int)n->panels.size(); i++) {
            float textW = f.MeasureW(n->panels[i].title.c_str());
            float dotW  = multiTab ? (DockNode::kTabDotR * 2.f + 8.f) : 0.f;
            float tw    = textW + DockNode::kTabPX * 2.f + dotW;

            // Clamp tab width to available area
            float avail = kTabEndX - tx;
            if (avail < 20.f) { n->tabWidths[i] = 0.f; continue; }
            if (tw > avail)   tw = avail;
            n->tabWidths[i] = tw;

            bool sel = (i == n->activeIdx);
            bool hov = (i == n->hoverTab);

            if (sel) {
                r.SetDrawColor(s.panelBg.r, s.panelBg.g, s.panelBg.b);
                r.FillRect(tx, n->y, tw, DockNode::kTabH);
            } else if (hov) {
                r.SetDrawColor(s.btnHover.r, s.btnHover.g, s.btnHover.b);
                r.FillRect(tx, n->y, tw, DockNode::kTabH);
            }

            // Title text
            Color tc = sel ? s.textBright : s.textDim;
            r.SetDrawColor(tc.r, tc.g, tc.b);
            f.DrawText(r, n->panels[i].title.c_str(),
                       tx + DockNode::kTabPX,
                       n->y + (DockNode::kTabH - f.GlyphH()) * 0.5f);

            // Per-tab ●/○ close dot (multi-panel only)
            if (multiTab) {
                float dx  = tx + tw - DockNode::kTabDotR - 5.f;
                float dy  = n->y + DockNode::kTabH * 0.5f;
                bool  dot = (i == n->hoverTabDot);
                if (dot) {
                    r.SetDrawColor(210, 80, 80);
                    r.FillRect(dx - DockNode::kTabDotR, dy - DockNode::kTabDotR,
                               DockNode::kTabDotR * 2.f, DockNode::kTabDotR * 2.f);
                } else if (sel) {
                    // filled ●
                    r.SetDrawColor(s.textDim.r, s.textDim.g, s.textDim.b);
                    r.FillRect(dx - DockNode::kTabDotR, dy - DockNode::kTabDotR,
                               DockNode::kTabDotR * 2.f, DockNode::kTabDotR * 2.f);
                } else {
                    // hollow ○
                    r.SetDrawColor(s.textDim.r, s.textDim.g, s.textDim.b);
                    r.DrawRect(dx - DockNode::kTabDotR, dy - DockNode::kTabDotR,
                               DockNode::kTabDotR * 2.f, DockNode::kTabDotR * 2.f);
                }
            }

            // Active accent bar
            if (sel) {
                r.SetDrawColor(s.highlight.r, s.highlight.g, s.highlight.b);
                r.FillRect(tx, n->y + DockNode::kTabH - 2.f, tw, 2.f);
            }

            // Tab separator
            r.SetDrawColor(s.separator.r, s.separator.g, s.separator.b);
            r.FillRect(tx + tw - 1.f, n->y, 1.f, DockNode::kTabH);
            tx += tw;
        }

        // ── Right-side buttons: — (minimize) and ● (close) ───────────────────
        {
            float bx  = n->x + n->w - kBtnAreaW;
            float mid = n->y + DockNode::kTabH * 0.5f;

            // — minimize
            if (n->hoverMinBtn) {
                r.SetDrawColor(s.btnHover.r, s.btnHover.g, s.btnHover.b);
                r.FillRect(bx, n->y, DockNode::kBtnW, DockNode::kTabH);
            }
            r.SetDrawColor(n->hoverMinBtn ? s.textBright.r : s.textDim.r,
                           n->hoverMinBtn ? s.textBright.g : s.textDim.g,
                           n->hoverMinBtn ? s.textBright.b : s.textDim.b);
            r.FillRect(bx + 3.f, mid - 1.f, DockNode::kBtnW - 6.f, 2.f);
            bx += DockNode::kBtnW + 2.f;

            // ● close
            if (n->hoverCloseBtn) {
                r.SetDrawColor(160, 40, 40);
                r.FillRect(bx, n->y, DockNode::kBtnW, DockNode::kTabH);
            }
            float cx = bx + DockNode::kBtnW * 0.5f;
            float cy = mid;
            r.SetDrawColor(n->hoverCloseBtn ? 240 : s.textDim.r,
                           n->hoverCloseBtn ?  80 : s.textDim.g,
                           n->hoverCloseBtn ?  80 : s.textDim.b);
            r.FillRect(cx - 3.5f, cy - 3.5f, 7.f, 7.f);
        }

        // Tab bar bottom border
        r.SetDrawColor(s.separator.r, s.separator.g, s.separator.b);
        r.FillRect(n->x, n->y + DockNode::kTabH - 1.f, n->w, 1.f);

        // ── Active panel content (hidden when minimized) ──────────────────────
        if (!n->minimized) {
            int ai = n->activeIdx;
            if (ai >= 0 && ai < (int)n->panels.size() && n->panels[ai].widget) {
                n->panels[ai].widget->visible = true;
                n->panels[ai].widget->Render(r, f, 0.f, 0.f);
            }
        }

        // Leaf border
        r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b);
        r.DrawRect(n->x, n->y, n->w, n->h);
    }

    // ── Drop zone overlay ─────────────────────────────────────────────────────
    void renderDropZones(DockNode* leaf, LightningEngine::Renderer& r, TitanFont& f)
    {
        // Full-leaf dim overlay
        r.SetDrawColor(38, 88, 195, 35);
        r.FillRect(leaf->x, leaf->y + DockNode::kTabH, leaf->w, leaf->h - DockNode::kTabH);

        float cx  = leaf->x + leaf->w * 0.5f;
        float cy  = leaf->y + DockNode::kTabH + (leaf->h - DockNode::kTabH) * 0.5f;
        float gap = 40.f;
        float sz  = 26.f;

        auto drawZone = [&](float bx, float by, DropZone id, const char* lbl) {
            bool hov = (dropZone == id);
            r.SetDrawColor(hov ? 60 : 30, hov ? 100 : 55, hov ? 200 : 130, hov ? 220 : 150);
            r.FillRect(bx - sz*0.5f, by - sz*0.5f, sz, sz);
            r.SetDrawColor(hov ? 140 : 80, hov ? 190 : 130, hov ? 255 : 200);
            r.DrawRect(bx - sz*0.5f, by - sz*0.5f, sz, sz);
            float gh = f.GlyphH(), lw = f.MeasureW(lbl);
            r.SetDrawColor(220, 235, 255);
            f.DrawText(r, lbl, bx - lw*0.5f, by - gh*0.5f);
        };

        drawZone(cx,       cy - gap, DZ_Top,    "T");
        drawZone(cx,       cy + gap, DZ_Bottom, "B");
        drawZone(cx - gap, cy,       DZ_Left,   "L");
        drawZone(cx + gap, cy,       DZ_Right,  "R");
        drawZone(cx,       cy,       DZ_Tab,    "+");
    }

    // ── Input: splitter handles ──────────────────────────────────────────────
    bool processNodeInput(DockNode* n, float mx, float my,
                          bool ldown, bool lclick, bool lrelease, TitanUI* ui)
    {
        if (!n || n->type != DockNode::Type::Split) return false;

        // Recurse first (deeper splits take priority)
        if (processNodeInput(n->childA.get(), mx, my, ldown, lclick, lrelease, ui)) return true;
        if (processNodeInput(n->childB.get(), mx, my, ldown, lclick, lrelease, ui)) return true;

        // Compute handle rect
        float hx, hy, hw, hh;
        if (n->splitV) {
            float mid = n->x + n->ratio * n->w;
            hx = mid - DockNode::kHandleW*0.5f; hy = n->y;
            hw = DockNode::kHandleW; hh = n->h;
        } else {
            float mid = n->y + n->ratio * n->h;
            hx = n->x; hy = mid - DockNode::kHandleW*0.5f;
            hw = n->w; hh = DockNode::kHandleW;
        }

        bool inHandle = mx >= hx && mx < hx+hw && my >= hy && my < hy+hh;
        n->hoverHandle = inHandle;

        if (lclick && inHandle) {
            n->draggHandle = true;
            n->dragOrigin  = n->splitV ? mx : my;
            n->dragRatio   = n->ratio;
            SetUICapture(ui, this);
            return true;
        }

        if (n->draggHandle) {
            float cur   = n->splitV ? mx : my;
            float total = n->splitV ? n->w : n->h;
            float delta = cur - n->dragOrigin;
            float nr    = n->dragRatio + delta / total;
            float minR  = 50.f / total;
            float maxR  = 1.f - 50.f / total;
            if (nr < minR) nr = minR;
            if (nr > maxR) nr = maxR;
            n->ratio = nr;
            n->computeChildGeometry();
            if (!ldown) {
                n->draggHandle = false;
                SetUICapture(ui, nullptr);
            }
            return true;
        }

        return false;
    }

    // ── Input: tab bars ──────────────────────────────────────────────────────
    bool processLeafTabs(DockNode* n, float mx, float my,
                         bool ldown, bool lclick, bool lrelease, TitanUI* ui)
    {
        if (!n) return false;
        if (n->type == DockNode::Type::Split) {
            if (processLeafTabs(n->childA.get(), mx, my, ldown, lclick, lrelease, ui)) return true;
            if (processLeafTabs(n->childB.get(), mx, my, ldown, lclick, lrelease, ui)) return true;
            return false;
        }
        if (n->panels.empty()) return false;

        bool inTabBar = mx >= n->x && mx < n->x + n->w
                     && my >= n->y && my < n->y + DockNode::kTabH;

        // Reset hover state
        n->hoverMinBtn   = false;
        n->hoverCloseBtn = false;
        n->hoverTabDot   = -1;
        if (!inTabBar) { n->hoverTab = -1; return false; }

        const float kBtnAreaW = DockNode::kBtnW * 2.f + 4.f;

        // ── ⋮ drag handle ─────────────────────────────────────────────────────
        if (mx < n->x + DockNode::kDragHandleW) {
            if (lclick && n->activeIdx < (int)n->panels.size() && n->panels[n->activeIdx].widget) {
                std::string title  = n->panels[n->activeIdx].title;
                Widget*     w      = n->panels[n->activeIdx].widget.get();
                float       offX   = mx - w->x, offY = my - w->y;
                auto        owned  = n->Undock(w);
                if (owned) {
                    AdoptToTitanUI(std::move(owned), ui);
                    BeginDrag(w, title.c_str(), mx, my, offX, offY);
                    if (n->panels.empty()) n->CollapseIfEmpty();
                    return true;
                }
            }
            return true;  // consume hover over handle
        }

        // ── Right-side buttons ─────────────────────────────────────────────────
        float btnX = n->x + n->w - kBtnAreaW;

        // — minimize
        if (mx >= btnX && mx < btnX + DockNode::kBtnW) {
            n->hoverMinBtn = true;
            if (lclick) { n->minimized = !n->minimized; n->applyPanelGeometry(); }
            return true;
        }
        btnX += DockNode::kBtnW + 2.f;

        // ● close active panel
        if (mx >= btnX && mx < btnX + DockNode::kBtnW) {
            n->hoverCloseBtn = true;
            if (lclick && n->activeIdx < (int)n->panels.size()) {
                auto owned = n->Undock(n->panels[n->activeIdx].widget.get());
                owned.reset();  // destroy panel
                if (n->panels.empty()) n->CollapseIfEmpty();
            }
            return true;
        }

        // ── Tab hit test ───────────────────────────────────────────────────────
        bool multiTab = (int)n->panels.size() > 1;
        n->hoverTab = -1;
        float tx = n->x + DockNode::kDragHandleW;

        for (int i = 0; i < (int)n->panels.size(); i++) {
            float tw = (i < (int)n->tabWidths.size()) ? n->tabWidths[i] : 0.f;
            if (tw <= 0.f) continue;

            if (mx >= tx && mx < tx + tw) {
                n->hoverTab = i;

                // Per-tab close dot (multi-panel only)
                if (multiTab) {
                    float dx = tx + tw - DockNode::kTabDotR - 5.f;
                    float dy = n->y + DockNode::kTabH * 0.5f;
                    float hr = DockNode::kTabDotR + 4.f;  // generous hit radius
                    if (std::abs(mx - dx) <= hr && std::abs(my - dy) <= hr) {
                        n->hoverTabDot = i;
                        if (lclick) {
                            auto owned = n->Undock(n->panels[i].widget.get());
                            owned.reset();
                            if (n->panels.empty()) n->CollapseIfEmpty();
                            return true;
                        }
                        return true;
                    }
                }

                if (lclick) {
                    n->activeIdx = i;
                    n->applyPanelGeometry();
                }
                return true;
            }
            tx += tw;
        }
        return inTabBar;
    }

    // ── Input: forward to active leaf panels ─────────────────────────────────
    bool forwardToLeaves(DockNode* n, float mx, float my,
                         bool ldown, bool lclick, bool lrelease, TitanUI* ui)
    {
        if (!n) return false;
        if (n->type == DockNode::Type::Split) {
            if (forwardToLeaves(n->childA.get(), mx, my, ldown, lclick, lrelease, ui)) return true;
            if (forwardToLeaves(n->childB.get(), mx, my, ldown, lclick, lrelease, ui)) return true;
            return false;
        }
        if (n->minimized) return false;  // minimized leaves don't receive panel input
        int ai = n->activeIdx;
        if (ai < 0 || ai >= (int)n->panels.size() || !n->panels[ai].widget) return false;
        Widget* w = n->panels[ai].widget.get();
        return w->ProcessInput(mx, my, ldown, lclick, lrelease, ui, 0.f, 0.f);
    }

    // ── Drop zone hit detection ───────────────────────────────────────────────
    DropZone hitDropZone(DockNode* leaf, float mx, float my)
    {
        float cx  = leaf->x + leaf->w * 0.5f;
        float cy  = leaf->y + DockNode::kTabH + (leaf->h - DockNode::kTabH) * 0.5f;
        float gap = 40.f, r = 16.f;
        auto hit = [&](float bx, float by) {
            return std::abs(mx-bx) < r && std::abs(my-by) < r;
        };
        if (hit(cx,       cy - gap)) return DZ_Top;
        if (hit(cx,       cy + gap)) return DZ_Bottom;
        if (hit(cx - gap, cy))       return DZ_Left;
        if (hit(cx + gap, cy))       return DZ_Right;
        if (hit(cx,       cy))       return DZ_Tab;
        return DZ_None;
    }

    // ── Re-dock ───────────────────────────────────────────────────────────────
    void performDock(Widget* w, const char* title, DockNode* target, DropZone zone, TitanUI* ui)
    {
        // Retrieve ownership from TitanUI
        auto owned = ReleaseFromTitanUI(w, ui);
        if (!owned) return;  // widget not found as a TitanUI root

        if (zone == DZ_Tab) {
            target->Dock(std::move(owned), title);
        } else {
            bool vert  = (zone == DZ_Left || zone == DZ_Right);
            bool first = (zone == DZ_Left  || zone == DZ_Top);
            auto [a, b] = target->Split(vert, first ? 0.35f : 0.65f);
            DockNode* newLeaf = first ? a : b;
            newLeaf->Dock(std::move(owned), title);
        }
    }

    // ── TitanUI ownership bridge ─────────────────────────────────────────────
    // These manipulate TitanUI::roots via the public ClearRoots-like mechanism.
    // We access TitanUI's AddRoot/release through the helpers below.
    // Since TitanUI is not included here (would be circular), we use the
    // Widget* zOrder trick and a deferred adoption list.
    //
    // Floating panels are stored in floatingOwned until TitanUI::ProcessInput
    // calls DockSpace::FlushAdoptions. For simplicity we store them here
    // and render them from DockSpace.

    std::vector<std::pair<std::string, std::unique_ptr<Widget>>> floating;

    void AdoptToTitanUI(std::unique_ptr<Widget> w, TitanUI* /*ui*/)
    {
        // Store in floating list — DockSpace renders these
        if (w) {
            w->zOrder = 9999;
            floating.push_back({ dragTitle, std::move(w) });
        }
    }

    std::unique_ptr<Widget> ReleaseFromTitanUI(Widget* target, TitanUI* /*ui*/)
    {
        for (auto it = floating.begin(); it != floating.end(); ++it) {
            if (it->second.get() == target) {
                auto owned = std::move(it->second);
                floating.erase(it);
                owned->zOrder = 0;
                return owned;
            }
        }
        return nullptr;
    }

    // Override Render to also draw floating panels
    // (called after renderNode in the main Render)
public:
    void RenderFloating(LightningEngine::Renderer& r, TitanFont& f)
    {
        for (auto& [title, w] : floating) {
            if (w) w->Render(r, f, 0.f, 0.f);
        }
    }

    bool ProcessInputFloating(float mx, float my, bool ldown, bool lclick, bool lrelease, TitanUI* ui)
    {
        for (int i = (int)floating.size()-1; i >= 0; i--) {
            if (floating[i].second) {
                if (floating[i].second->ProcessInput(mx, my, ldown, lclick, lrelease, ui, 0.f, 0.f))
                    return true;
            }
        }
        return false;
    }
};

} // namespace Titan
