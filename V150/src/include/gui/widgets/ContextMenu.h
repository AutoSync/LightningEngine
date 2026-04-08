// ContextMenu.h — Popup context menu triggered at an arbitrary screen position.
// Add items with AddItem(). Call Open(x, y) to show at position.
// Closes automatically when an item is clicked or when clicking outside.
//
// Usage:
//   auto* ctx = ui.AddRoot<ContextMenu>();  // add once as root
//   ctx->AddItem("Cut",   [](){ ... });
//   ctx->AddItem("Copy",  [](){ ... });
//   ctx->AddItem("Paste", [](){ ... });
//   ctx->AddSeparator();
//   ctx->AddItem("Delete", [](){ ... });
//
//   // On right-click:
//   if (im.IsMousePressed(3)) ctx->Open(im.GetMouseX(), im.GetMouseY());
#pragma once
#include <algorithm>
#include <cctype>
#include <vector>
#include <string>
#include <functional>
#include "../Widget.h"
#include "../TitanStyle.h"
#include "../TitanFont.h"

namespace Titan {

class ContextMenu : public Widget {
public:
    struct Item {
        std::string text;
        bool        separator = false;
        bool        enabled   = true;
        std::function<void()> action;
    };

    std::vector<Item> items;
    int   hoverIndex = -1;

    static constexpr float kItemH = 20.f;
    static constexpr float kSepH  = 5.f;
    static constexpr float kMinW  = 120.f;
    static constexpr float kPadX  = 10.f;

    ContextMenu() { visible = false; zOrder = 9000; }

    void AddItem(const char* text, std::function<void()> action = nullptr,
                 bool enabled = true)
    {
        items.push_back({ text ? text : "", false, enabled, std::move(action) });
        treeDirty = true;
    }

    void AddSeparator()
    {
        items.push_back({ "", true, false, nullptr });
        treeDirty = true;
    }

    void Open(float px, float py)
    {
        x = px; y = py;
        visible = true;
        hoverIndex = -1;
        openPath.clear();
        hoverPath.clear();
        scrollByLevel.clear();
        treeDirty = true;
        rebuildSize();
    }

    void Close() {
        visible = false;
        hoverIndex = -1;
        openPath.clear();
        hoverPath.clear();
        scrollByLevel.clear();
    }

    bool ProcessInput(float mx, float my, bool ldown, bool lclick, bool lrelease,
                      TitanUI* ui, float ox, float oy) override
    {
        if (!visible) return false;
        ensureTree();

        if (cachedFont) rebuildWindowLayout(*cachedFont);

        int hoveredWindow = -1;
        for (int wi = 0; wi < (int)windows.size(); ++wi) {
            const MenuWindow& wnd = windows[wi];
            float ax = ox + wnd.x;
            float ay = oy + wnd.y;
            if (mx >= ax && mx < ax + wnd.w && my >= ay && my < ay + wnd.h) {
                hoveredWindow = wi;
                break;
            }
        }

        if (hoveredWindow < 0) {
            if (lclick) Close();
            return false;
        }

        // Wheel scroll on the hovered menu window when content exceeds viewport.
        float wheelY = GetUIScrollY(ui);
        if (wheelY != 0.f && hoveredWindow < (int)windows.size()) {
            MenuWindow& wnd = windows[hoveredWindow];
            float maxScroll = std::max(0.f, wnd.contentH - wnd.h);
            if (maxScroll > 0.f) {
                float next = wnd.scroll - wheelY * (kItemH * 1.5f);
                wnd.scroll = std::clamp(next, 0.f, maxScroll);
                if (hoveredWindow < (int)scrollByLevel.size())
                    scrollByLevel[hoveredWindow] = wnd.scroll;
            }
        }

        // Reset hover path and keep only the submenu chain up to hovered window.
        hoverPath.assign(windows.size(), -1);
        if ((int)openPath.size() > hoveredWindow) openPath.resize(hoveredWindow);

        const std::vector<MenuNode>* entries = entriesForLevel(hoveredWindow);
        if (!entries) return true;

        const MenuWindow& wnd = windows[hoveredWindow];
        float iy = oy + wnd.y - wnd.scroll;
        int hoveredEntry = -1;

        for (int i = 0; i < (int)entries->size(); ++i) {
            float ih = (*entries)[i].separator ? kSepH : kItemH;
            if (!(*entries)[i].separator && my >= iy && my < iy + ih &&
                iy < (oy + wnd.y + wnd.h) && (iy + ih) > (oy + wnd.y)) {
                if ((*entries)[i].enabled) hoveredEntry = i;
                break;
            }
            iy += ih;
        }

        if (hoveredEntry >= 0) {
            hoverPath[hoveredWindow] = hoveredEntry;
            const MenuNode& entry = (*entries)[hoveredEntry];
            if (!entry.children.empty()) {
                if ((int)openPath.size() == hoveredWindow)
                    openPath.push_back(hoveredEntry);
                else
                    openPath[hoveredWindow] = hoveredEntry;
                if ((int)scrollByLevel.size() <= hoveredWindow + 1)
                    scrollByLevel.resize((size_t)hoveredWindow + 2, 0.f);
            } else if ((int)openPath.size() > hoveredWindow) {
                openPath.resize(hoveredWindow);
                if ((int)scrollByLevel.size() > hoveredWindow + 1)
                    scrollByLevel.resize((size_t)hoveredWindow + 1);
            }

            if (lclick) {
                if (entry.enabled && entry.action) entry.action();
                Close();
                return true;
            }
        } else if ((int)openPath.size() > hoveredWindow) {
            openPath.resize(hoveredWindow);
            if ((int)scrollByLevel.size() > hoveredWindow + 1)
                scrollByLevel.resize((size_t)hoveredWindow + 1);
        }

        // Keep legacy field updated for callers that still inspect it.
        hoverIndex = (!hoverPath.empty()) ? hoverPath[0] : -1;
        return true; // consume all input while open
    }

    void Render(LightningEngine::Renderer& r, TitanFont& f, float ox, float oy) override
    {
        if (!visible) return;
        ensureTree();
        cachedFont = &f;
        rebuildWindowLayout(f);

        const Style& s = gStyle;
        for (int wi = 0; wi < (int)windows.size(); ++wi) {
            const MenuWindow& wnd = windows[wi];
            const std::vector<MenuNode>* entries = entriesForLevel(wi);
            if (!entries) continue;

            float ax = ox + wnd.x;
            float ay = oy + wnd.y;

            // Shadow
            r.SetDrawColor(0, 0, 0, 100);
            r.FillRect(ax + 3.f, ay + 3.f, wnd.w, wnd.h);

            // Background
            r.SetDrawColor(s.menuBg.r, s.menuBg.g, s.menuBg.b);
            r.FillRect(ax, ay, wnd.w, wnd.h);

            r.SetScissor(ax, ay, wnd.w, wnd.h);

            float iy = ay - wnd.scroll;
            int hi = (wi < (int)hoverPath.size()) ? hoverPath[wi] : -1;
            if (hi < 0 && wi < (int)openPath.size()) hi = openPath[wi];

            for (int i = 0; i < (int)entries->size(); ++i) {
                const MenuNode& entry = (*entries)[i];

                if (entry.separator) {
                    float sy = iy + kSepH * 0.5f;
                    r.SetDrawColor(s.separator.r, s.separator.g, s.separator.b);
                    r.FillRect(ax + kPadX, sy, wnd.w - kPadX * 2.f, 1.f);
                    iy += kSepH;
                    continue;
                }

                if (i == hi) {
                    r.SetDrawColor(s.highlight.r, s.highlight.g, s.highlight.b);
                    r.FillRect(ax + 2.f, iy, wnd.w - 4.f, kItemH);
                }

                Color tc = !entry.enabled ? s.textDim
                         : (i == hi ? s.textBright : s.textNormal);
                r.SetDrawColor(tc.r, tc.g, tc.b);
                f.DrawText(r, entry.text.c_str(),
                           ax + kPadX,
                           iy + (kItemH - f.GlyphH()) * 0.5f);

                if (!entry.children.empty()) {
                    const char* arrow = ">";
                    float aw = f.MeasureW(arrow);
                    f.DrawText(r, arrow,
                               ax + wnd.w - kPadX - aw,
                               iy + (kItemH - f.GlyphH()) * 0.5f);
                }

                iy += kItemH;
            }

            r.ClearScissor();

            // Scroll cues
            if (wnd.contentH > wnd.h) {
                if (wnd.scroll > 0.5f) {
                    r.SetDrawColor(s.separator.r, s.separator.g, s.separator.b, 220);
                    r.FillRect(ax + wnd.w - 12.f, ay + 4.f, 6.f, 2.f);
                }
                if (wnd.scroll < (wnd.contentH - wnd.h - 0.5f)) {
                    r.SetDrawColor(s.separator.r, s.separator.g, s.separator.b, 220);
                    r.FillRect(ax + wnd.w - 12.f, ay + wnd.h - 6.f, 6.f, 2.f);
                }
            }

            // Border
            r.SetDrawColor(s.menuBorder.r, s.menuBorder.g, s.menuBorder.b);
            r.DrawRect(ax, ay, wnd.w, wnd.h);
        }
    }

private:
    struct MenuNode {
        std::string text;
        bool separator = false;
        bool enabled = true;
        std::function<void()> action;
        std::vector<MenuNode> children;
    };

    struct MenuWindow {
        float x = 0.f;
        float y = 0.f;
        float w = 0.f;
        float h = 0.f;
        float contentH = 0.f;
        float scroll = 0.f;
    };

    bool treeDirty = true;
    TitanFont* cachedFont = nullptr;
    std::vector<MenuNode> rootEntries;
    std::vector<int> openPath;
    std::vector<int> hoverPath;
    std::vector<float> scrollByLevel;
    std::vector<MenuWindow> windows;

    static std::string trimToken(const std::string& token)
    {
        size_t start = 0;
        while (start < token.size() && std::isspace((unsigned char)token[start])) ++start;

        size_t end = token.size();
        while (end > start && std::isspace((unsigned char)token[end - 1])) --end;
        return token.substr(start, end - start);
    }

    static std::vector<std::string> splitPath(const std::string& text)
    {
        std::vector<std::string> out;
        if (text.empty()) return out;

        size_t start = 0;
        while (start <= text.size()) {
            size_t slash = text.find('/', start);
            std::string piece = (slash == std::string::npos)
                ? text.substr(start)
                : text.substr(start, slash - start);
            piece = trimToken(piece);
            if (!piece.empty()) out.push_back(piece);

            if (slash == std::string::npos) break;
            start = slash + 1;
        }
        return out;
    }

    void ensureTree()
    {
        if (!treeDirty) return;

        rootEntries.clear();
        for (const Item& item : items) {
            if (item.separator) {
                rootEntries.push_back({ "", true, false, nullptr, {} });
                continue;
            }

            std::vector<std::string> path = splitPath(item.text);
            if (path.empty()) continue;

            std::vector<MenuNode>* bucket = &rootEntries;
            for (size_t i = 0; i < path.size(); ++i) {
                const bool isLeaf = (i + 1 == path.size());
                const std::string& label = path[i];

                auto found = std::find_if(bucket->begin(), bucket->end(),
                    [&label](const MenuNode& node) {
                        return !node.separator && node.text == label;
                    });

                if (found == bucket->end()) {
                    bucket->push_back({ label, false, true, nullptr, {} });
                    found = std::prev(bucket->end());
                }

                if (isLeaf) {
                    found->enabled = item.enabled;
                    found->action = item.action;
                }

                bucket = &found->children;
            }
        }

        treeDirty = false;
    }

    const std::vector<MenuNode>* entriesForLevel(int level) const
    {
        if (level < 0) return nullptr;
        if (level == 0) return &rootEntries;

        const std::vector<MenuNode>* current = &rootEntries;
        for (int d = 0; d < level; ++d) {
            if (d >= (int)openPath.size()) return nullptr;
            int idx = openPath[d];
            if (idx < 0 || idx >= (int)current->size()) return nullptr;

            const MenuNode& node = (*current)[idx];
            if (node.children.empty()) return nullptr;
            current = &node.children;
        }
        return current;
    }

    float menuHeight(const std::vector<MenuNode>& entries) const
    {
        float totalH = 0.f;
        for (const MenuNode& it : entries)
            totalH += it.separator ? kSepH : kItemH;
        return totalH;
    }

    float menuWidth(const std::vector<MenuNode>& entries, TitanFont& f) const
    {
        float maxW = kMinW;
        for (const MenuNode& it : entries) {
            if (it.separator) continue;
            float tw = f.MeasureW(it.text.c_str()) + kPadX * 2.f;
            if (!it.children.empty()) tw += f.MeasureW(">") + 10.f;
            if (tw > maxW) maxW = tw;
        }
        return maxW;
    }

    float itemOffsetY(const std::vector<MenuNode>& entries, int itemIndex) const
    {
        float yOffset = 0.f;
        for (int i = 0; i < itemIndex && i < (int)entries.size(); ++i)
            yOffset += entries[i].separator ? kSepH : kItemH;
        return yOffset;
    }

    void windowSize(float& sw, float& sh) const
    {
        sw = 1280.f;
        sh = 720.f;
        SDL_Window* win = SDL_GetMouseFocus();
        if (!win) win = SDL_GetKeyboardFocus();
        if (!win) return;
        int wPx = 0, hPx = 0;
        SDL_GetWindowSize(win, &wPx, &hPx);
        if (wPx > 0) sw = (float)wPx;
        if (hPx > 0) sh = (float)hPx;
    }

    void rebuildWindowLayout(TitanFont& f)
    {
        windows.clear();

        const std::vector<MenuNode>* root = entriesForLevel(0);
        if (!root || root->empty()) {
            w = kMinW;
            h = 0.f;
            return;
        }

        float sw = 1280.f, sh = 720.f;
        windowSize(sw, sh);
        const float maxMenuH = std::max(120.f, sh - 8.f);

        if (scrollByLevel.empty()) scrollByLevel.push_back(0.f);

        MenuWindow rootWnd;
        rootWnd.w = menuWidth(*root, f);
        rootWnd.contentH = menuHeight(*root);
        rootWnd.h = std::min(rootWnd.contentH, maxMenuH);
        rootWnd.scroll = std::clamp(scrollByLevel[0], 0.f, std::max(0.f, rootWnd.contentH - rootWnd.h));
        scrollByLevel[0] = rootWnd.scroll;
        rootWnd.x = std::clamp(x, 0.f, std::max(0.f, sw - rootWnd.w));
        rootWnd.y = std::clamp(y, 0.f, std::max(0.f, sh - rootWnd.h));
        windows.push_back(rootWnd);

        for (int depth = 0; depth < (int)openPath.size(); ++depth) {
            const std::vector<MenuNode>* parentEntries = entriesForLevel(depth);
            if (!parentEntries) break;
            int idx = openPath[depth];
            if (idx < 0 || idx >= (int)parentEntries->size()) break;

            const MenuNode& parentItem = (*parentEntries)[idx];
            if (parentItem.children.empty()) break;

            const MenuWindow& parentWnd = windows[depth];

            MenuWindow subWnd;
            subWnd.w = menuWidth(parentItem.children, f);
            subWnd.contentH = menuHeight(parentItem.children);
            subWnd.h = std::min(subWnd.contentH, maxMenuH);

            if ((int)scrollByLevel.size() <= depth + 1)
                scrollByLevel.resize((size_t)depth + 2, 0.f);
            subWnd.scroll = std::clamp(scrollByLevel[depth + 1], 0.f,
                                       std::max(0.f, subWnd.contentH - subWnd.h));
            scrollByLevel[depth + 1] = subWnd.scroll;

            float itemY = parentWnd.y + itemOffsetY(*parentEntries, idx) - parentWnd.scroll;
            float rightX = parentWnd.x + parentWnd.w;
            float leftX = parentWnd.x - subWnd.w;
            subWnd.x = (rightX + subWnd.w <= sw) ? rightX : leftX;
            subWnd.y = itemY;

            subWnd.x = std::clamp(subWnd.x, 0.f, std::max(0.f, sw - subWnd.w));
            subWnd.y = std::clamp(subWnd.y, 0.f, std::max(0.f, sh - subWnd.h));

            windows.push_back(subWnd);
        }

        if ((int)scrollByLevel.size() > (int)windows.size())
            scrollByLevel.resize(windows.size());

        // Keep widget bounds aligned to the root window for compatibility.
        x = rootWnd.x;
        y = rootWnd.y;
        w = rootWnd.w;
        h = rootWnd.h;
    }

    void rebuildSize()
    {
        ensureTree();
        float totalH = 0.f;
        for (const auto& it : rootEntries)
            totalH += it.separator ? kSepH : kItemH;
        h = totalH;
        w = kMinW;
    }

    void rebuildSizeWithFont(TitanFont& f)
    {
        ensureTree();
        w = menuWidth(rootEntries, f);
        h = menuHeight(rootEntries);
    }
};

} // namespace Titan
