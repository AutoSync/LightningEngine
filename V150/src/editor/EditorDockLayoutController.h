// EditorDockLayoutController.h — Chooses dock layout mode from persisted state.
#pragma once

#include "EditorLayoutState.h"

enum class EditorDockLayoutMode {
    FocusViewport,
    StandardNoBottom,
    StandardWithBottom,
};

struct EditorDockLayoutController {
    static EditorDockLayoutMode Resolve(const EditorLayoutState& state) {
        if (state.focusViewportMode) return EditorDockLayoutMode::FocusViewport;
        if (!state.bottomTrayVisible) return EditorDockLayoutMode::StandardNoBottom;
        return EditorDockLayoutMode::StandardWithBottom;
    }
};
