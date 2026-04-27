// IUiHost.h — Pure interface for the Lightning Engine UI subsystem (Titan).
//
// Plugins and gameplay code can add/remove widgets and drive the UI loop
// without including the full TitanUI.h tree.
//
// See: src/include/gui/TitanUI.h (concrete Titan implementation).
// See: docs/developers/01-architecture.md
#pragma once

#include <memory>
#include <string>

namespace LightningEngine {
    class IRenderer;
    class IInputManager;
}

namespace Titan {

class Widget;

class IUiHost {
public:
    virtual ~IUiHost() = default;

    // ── Lifecycle ────────────────────────────────────────────────────────
    // Process input and update internal widget state.
    virtual void Update(LightningEngine::IInputManager& input) = 0;
    // Render all root widgets via the renderer.
    virtual void Render(LightningEngine::IRenderer& renderer)  = 0;

    // ── Widget tree ──────────────────────────────────────────────────────
    // Add a root widget (UI host takes ownership).
    virtual void Add(std::unique_ptr<Widget> widget) = 0;
    // Remove a previously added root widget by pointer.
    virtual void Remove(const Widget* widget)        = 0;
    // Remove all root widgets.
    virtual void Clear()                             = 0;

    // ── Focus / capture ──────────────────────────────────────────────────
    virtual Widget* GetFocused()  const = 0;
    virtual Widget* GetCaptured() const = 0;
    virtual void    SetFocused (Widget* w) = 0;
    virtual void    SetCaptured(Widget* w) = 0;
};

} // namespace Titan
