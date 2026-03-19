// Widget.h — Titan GUI base widget.
#pragma once
#include <vector>
#include <memory>
#include <string>
#include <functional>
#include "../Renderer.h"
#include <SDL3/SDL.h>

namespace Titan {

class TitanUI;
class TitanFont;

class Widget {
public:
    float x=0, y=0, w=0, h=0;
    bool  visible=true, enabled=true;
    int   zOrder = 0;   // root-level only: higher = renders on top, receives input first
    std::string id;
    std::string tooltipText;

    // Set by SetUIFocus — used by destructor to clear dangling focused pointer.
    TitanUI* uiContext = nullptr;

    std::vector<std::unique_ptr<Widget>> children;

    virtual ~Widget();          // defined after ClearUIFocusIfMatch in TitanUI.h
    void DoBringToFront(TitanUI* ui); // defined after TitanUI class in TitanUI.h

    bool Contains(float mx, float my, float ox, float oy) const {
        return mx >= ox+x && mx < ox+x+w && my >= oy+y && my < oy+y+h;
    }

    virtual bool ProcessInput(float mx, float my, bool ldown, bool lclick, bool lrelease,
                              TitanUI* ui, float ox, float oy)
    {
        if (!visible || !enabled) return false;
        for (int i = (int)children.size()-1; i >= 0; i--) {
            if (children[i]->ProcessInput(mx, my, ldown, lclick, lrelease, ui, ox+x, oy+y))
                return true;
        }
        return false;
    }

    virtual void Render(LightningEngine::Renderer& r, TitanFont& f, float ox, float oy)
    {
        if (!visible) return;
        for (auto& c : children) c->Render(r, f, ox+x, oy+y);
    }

    template<typename T, typename... Args>
    T* Add(Args&&... args)
    {
        auto w = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = w.get();
        children.push_back(std::move(w));
        return ptr;
    }

    void Clear() { children.clear(); }

    // Returns the tooltip text of the deepest visible widget under (mx,my), or nullptr.
    virtual const std::string* FindTooltip(float mx, float my, float ox, float oy) const {
        if (!visible) return nullptr;
        float ax = ox+x, ay = oy+y;
        if (mx < ax || mx >= ax+w || my < ay || my >= ay+h) return nullptr;
        for (int i = (int)children.size()-1; i >= 0; i--) {
            const std::string* t = children[i]->FindTooltip(mx, my, ax, ay);
            if (t && !t->empty()) return t;
        }
        return tooltipText.empty() ? nullptr : &tooltipText;
    }

    // Called by TitanUI when keyboard text arrives while this widget has focus.
    virtual void ReceiveText(const std::string& txt, bool backspace) {}

    // Called by TitanUI when a navigation key is pressed while this widget has focus.
    // Keys: SDL_SCANCODE_LEFT/RIGHT/UP/DOWN, HOME, END, RETURN, TAB, PAGEUP, PAGEDOWN.
    virtual void ReceiveKey(SDL_Scancode key) {}

    // Called by SetUIFocus when this widget loses focus to another widget.
    // Override to clear visual focus state (e.g. isFocused = false).
    virtual void OnLostFocus() {}
};

// Helpers declared here; implemented inline in TitanUI.h after TitanUI is complete.
void SetUICapture(TitanUI* ui, Widget* w);
void SetUIFocus(TitanUI* ui, Widget* w);
float GetUIScrollY(TitanUI* ui);
void ClearUIFocusIfMatch(TitanUI* ui, Widget* w);  // called by ~Widget()

} // namespace Titan
