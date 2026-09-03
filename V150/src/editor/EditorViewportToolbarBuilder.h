// EditorViewportToolbarBuilder.h — Builds viewport-related toolbar controls.
#pragma once

#include <functional>
#include "EditorLayoutState.h"
#include "../include/gui/widgets/Toolbar.h"

struct EditorViewportToolbarCallbacks {
    std::function<void(bool)> onGridToggle;
    std::function<void(bool)> onSnapToggle;
    std::function<void(bool)> onBottomPanelToggle;
    std::function<void(bool)> onFocusViewToggle;
    std::function<void()> onFrameSelection;
    std::function<void()> onResetView;
};

struct EditorViewportToolbarBuilder {
    static void Append(Titan::Toolbar* toolbar,
                       const EditorLayoutState& state,
                       bool showGrid,
                       bool snapToGrid,
                       const EditorViewportToolbarCallbacks& callbacks) {
        if (!toolbar) return;

        toolbar->AddSeparator();

        toolbar->AddButton("Grid", [callbacks](bool active) {
            if (callbacks.onGridToggle) callbacks.onGridToggle(active);
        }, true);
        toolbar->items.back().active = showGrid;

        toolbar->AddButton("Snap", [callbacks](bool active) {
            if (callbacks.onSnapToggle) callbacks.onSnapToggle(active);
        }, true);
        toolbar->items.back().active = snapToGrid;

        toolbar->AddButton("Bottom Panel", [callbacks](bool active) {
            if (callbacks.onBottomPanelToggle) callbacks.onBottomPanelToggle(active);
        }, true);
        toolbar->items.back().active = state.bottomTrayVisible;

        toolbar->AddButton("Focus View", [callbacks](bool active) {
            if (callbacks.onFocusViewToggle) callbacks.onFocusViewToggle(active);
        }, true);
        toolbar->items.back().active = state.focusViewportMode;

        toolbar->AddButton("Enquadrar", [callbacks](bool) {
            if (callbacks.onFrameSelection) callbacks.onFrameSelection();
        }, false);

        toolbar->AddButton("Reset View", [callbacks](bool) {
            if (callbacks.onResetView) callbacks.onResetView();
        }, false);
    }
};
