// TreeView.h — Collapsible tree widget.
#pragma once
#include <vector>
#include <memory>
#include <string>
#include <functional>
#include <algorithm>
#include "../Widget.h"
#include "../TitanStyle.h"
#include "../TitanFont.h"

namespace Titan {

struct TreeNode {
    std::string label;
    std::string tag;        // shown dim, right-aligned
    void*       userData = nullptr;
    bool        expanded = false;
    std::vector<std::unique_ptr<TreeNode>> children;

    TreeNode() = default;
    TreeNode(const std::string& label, const std::string& tag = "")
        : label(label), tag(tag) {}

    TreeNode* AddChild(const std::string& lbl, const std::string& t = "")
    {
        auto n = std::make_unique<TreeNode>(lbl, t);
        TreeNode* ptr = n.get();
        children.push_back(std::move(n));
        return ptr;
    }

    void RemoveChild(TreeNode* node)
    {
        children.erase(
            std::remove_if(children.begin(), children.end(),
                [node](const std::unique_ptr<TreeNode>& c){ return c.get() == node; }),
            children.end());
    }

    void ClearChildren() { children.clear(); }
};

class TreeView : public Widget {
public:
    TreeNode  root;
    bool      showRoot      = false;
    float     indentW       = 14.f;
    float     itemH         = 0.f;   // 0 = use gStyle.lineH
    float     scrollOffset  = 0.f;
    TreeNode* selected      = nullptr;

    std::function<void(TreeNode*)> onSelect;

    static constexpr float kScrollW   = 8.f;
    static constexpr float kArrowW    = 10.f;
    static constexpr float kWheelSpd  = 20.f;

private:
    struct FlatItem { TreeNode* node; int depth; };
    mutable std::vector<FlatItem> flat;

    // Scrollbar drag state
    bool  sbDragging     = false;
    float sbDragStartY   = 0.f;
    float sbDragStartOff = 0.f;

    // Hover tracking
    mutable int hoverIdx = -1;

    void buildFlat() const
    {
        flat.clear();
        if (showRoot) {
            flat.push_back({const_cast<TreeNode*>(&root), 0});
            if (root.expanded) appendChildren(root, 1);
        } else {
            appendChildren(root, 0);
        }
    }

    void appendChildren(const TreeNode& node, int depth) const
    {
        for (auto& c : node.children) {
            flat.push_back({c.get(), depth});
            if (c->expanded) appendChildren(*c, depth + 1);
        }
    }

    float effectiveItemH() const
    {
        return (itemH > 0.f) ? itemH : gStyle.lineH;
    }

    float totalContentH() const
    {
        return flat.size() * effectiveItemH();
    }

    void clampScroll()
    {
        float maxOff = totalContentH() > h ? totalContentH() - h : 0.f;
        if (scrollOffset < 0.f)    scrollOffset = 0.f;
        if (scrollOffset > maxOff) scrollOffset = maxOff;
    }

public:
    TreeView() = default;
    TreeView(float x, float y, float w, float h)
    { this->x=x; this->y=y; this->w=w; this->h=h; }

    bool ProcessInput(float mx, float my, bool ldown, bool lclick, bool lrelease,
                      TitanUI* ui, float ox, float oy) override
    {
        if (!visible || !enabled) return false;
        float ax = ox+x, ay = oy+y;
        bool inView = mx >= ax && mx < ax+w && my >= ay && my < ay+h;

        buildFlat();
        float iH      = effectiveItemH();
        float contH   = totalContentH();
        float visW    = (contH > h) ? w - kScrollW : w;

        // Mouse wheel
        if (inView && ui && GetUIScrollY(ui) != 0.f) {
            scrollOffset -= GetUIScrollY(ui) * kWheelSpd;
            clampScroll();
        }

        // Scrollbar dragging
        if (contH > h) {
            float sbX    = ax + w - kScrollW;
            float maxOff = contH - h;
            float thumbH = h * (h / contH);
            if (thumbH < 12.f) thumbH = 12.f;
            float thumbY = ay + (maxOff > 0.f ? (scrollOffset / maxOff) : 0.f) * (h - thumbH);

            bool inThumb = mx >= sbX && mx < sbX + kScrollW
                        && my >= thumbY && my < thumbY + thumbH;

            if (lclick && inThumb) {
                sbDragging     = true;
                sbDragStartY   = my;
                sbDragStartOff = scrollOffset;
                SetUICapture(ui, this);
            }
            if (sbDragging) {
                float delta = my - sbDragStartY;
                float scale = (h - thumbH) > 0.f ? maxOff / (h - thumbH) : 0.f;
                scrollOffset = sbDragStartOff + delta * scale;
                clampScroll();
                if (!ldown) { sbDragging = false; SetUICapture(ui, nullptr); }
                return true;
            }
        }

        // Compute hover
        hoverIdx = -1;
        if (inView) {
            float relY = my - (ay - scrollOffset);
            int idx = (int)(relY / iH);
            if (idx >= 0 && idx < (int)flat.size()) {
                float itemTop = ay - scrollOffset + idx * iH;
                if (itemTop < ay + h && itemTop + iH > ay)
                    hoverIdx = idx;
            }
        }

        // Click interactions
        if (lclick && inView) {
            float relY = my - (ay - scrollOffset);
            int idx = (int)(relY / iH);
            if (idx >= 0 && idx < (int)flat.size()) {
                float itemTop = ay - scrollOffset + idx * iH;
                if (itemTop >= ay && itemTop + iH <= ay + h + iH) {
                    TreeNode* node = flat[idx].node;
                    int depth      = flat[idx].depth;
                    float indX     = ax + depth * indentW;
                    float arrowX   = indX;

                    // Arrow click zone
                    if (!node->children.empty() &&
                        mx >= arrowX && mx < arrowX + kArrowW)
                    {
                        node->expanded = !node->expanded;
                        return true;
                    }
                    // Label click
                    selected = node;
                    if (onSelect) onSelect(node);
                    return true;
                }
            }
            return true; // consume click inside widget
        }

        return inView && ldown;
    }

    void Render(LightningEngine::Renderer& r, TitanFont& f, float ox, float oy) override
    {
        if (!visible) return;
        float ax = ox+x, ay = oy+y;
        const Style& s = gStyle;

        buildFlat();
        float iH    = effectiveItemH();
        float contH = totalContentH();

        clampScroll();

        // Background
        r.SetDrawColor(s.panelBg.r, s.panelBg.g, s.panelBg.b);
        r.FillRect(ax, ay, w, h);

        float visW = (contH > h) ? w - kScrollW : w;

        // Items
        for (int i = 0; i < (int)flat.size(); i++) {
            float itemTop = ay - scrollOffset + i * iH;
            if (itemTop + iH < ay) continue;
            if (itemTop > ay + h)  break;

            TreeNode* node = flat[i].node;
            int depth      = flat[i].depth;
            float indX     = ax + depth * indentW;

            // Row highlight
            if (node == selected) {
                r.SetDrawColor(s.highlight.r, s.highlight.g, s.highlight.b);
                r.FillRect(ax, itemTop, visW, iH);
            } else if (i == hoverIdx) {
                r.SetDrawColor(s.selection.r, s.selection.g, s.selection.b, s.selection.a);
                r.FillRect(ax, itemTop, visW, iH);
            }

            float midY = itemTop + (iH - 6.f) * 0.5f;

            // Arrow / leaf indicator
            if (!node->children.empty()) {
                r.SetDrawColor(s.textDim.r, s.textDim.g, s.textDim.b);
                if (node->expanded) {
                    // Down triangle: ▼ (3×3 approximation via FillRect)
                    r.FillRect(indX + 1.f, midY,     6.f, 2.f);
                    r.FillRect(indX + 2.f, midY+2.f, 4.f, 2.f);
                    r.FillRect(indX + 3.f, midY+4.f, 2.f, 1.f);
                } else {
                    // Right triangle: ▶
                    r.FillRect(indX,       midY,     2.f, 6.f);
                    r.FillRect(indX + 2.f, midY+1.f, 2.f, 4.f);
                    r.FillRect(indX + 4.f, midY+2.f, 2.f, 2.f);
                }
            } else {
                // Leaf dash
                r.SetDrawColor(s.textDim.r, s.textDim.g, s.textDim.b);
                r.FillRect(indX + 1.f, midY + 2.f, 5.f, 2.f);
            }

            // Label
            float labelX = indX + kArrowW + 2.f;
            float labelY = itemTop + (iH - f.GlyphH()) * 0.5f;
            Color tc = (node == selected) ? s.textBright : s.textNormal;
            r.SetDrawColor(tc.r, tc.g, tc.b);
            f.DrawText(r, node->label.c_str(), labelX, labelY);

            // Tag (right-aligned, dim)
            if (!node->tag.empty()) {
                float tagW = f.MeasureW(node->tag.c_str());
                float tagX = ax + visW - tagW - s.padding;
                if (tagX > labelX + f.MeasureW(node->label.c_str()) + 4.f) {
                    r.SetDrawColor(s.textDim.r, s.textDim.g, s.textDim.b);
                    f.DrawText(r, node->tag.c_str(), tagX, labelY);
                }
            }
        }

        // Border
        r.SetDrawColor(s.panelBorder.r, s.panelBorder.g, s.panelBorder.b);
        r.DrawRect(ax, ay, w, h);

        // Scrollbar
        if (contH > h) {
            float maxOff = contH - h;
            float thumbH = h * (h / contH);
            if (thumbH < 12.f) thumbH = 12.f;
            float thumbY = ay + (maxOff > 0.f ? (scrollOffset / maxOff) : 0.f) * (h - thumbH);
            float sbX    = ax + w - kScrollW;

            r.SetDrawColor(s.sliderTrack.r, s.sliderTrack.g, s.sliderTrack.b);
            r.FillRect(sbX, ay, kScrollW, h);

            Color tc = sbDragging ? s.sliderFill : s.sliderThumb;
            r.SetDrawColor(tc.r, tc.g, tc.b);
            r.FillRect(sbX + 1.f, thumbY, kScrollW - 2.f, thumbH);
        }
    }
};

} // namespace Titan
