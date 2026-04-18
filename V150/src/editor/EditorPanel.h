// EditorPanel.h — Abstract base class for editor panels.
// Each panel owns its own UI widgets and local state,
// receives an EditorContext& for shared state access.
#pragma once
#include "EditorContext.h"
#include <memory>

class EditorPanel
{
public:
    virtual ~EditorPanel() = default;

    // Build the panel widget tree. Returns the root widget for docking.
    virtual std::unique_ptr<Widget> Build() = 0;

    // Refresh internal UI (e.g. after selection change).
    virtual void Refresh() {}

    // Per-frame update (input processing, animations).
    virtual void Update(float /*dt*/) {}

    // Called on window resize / layout change.
    virtual void OnResize(float /*w*/, float /*h*/) {}

    // Nullify all raw UI pointers (called when UI tree is rebuilt).
    virtual void ClearPtrs() {}

    // Release owned resources.
    virtual void Shutdown() {}
};
