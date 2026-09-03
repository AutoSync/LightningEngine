// EditorLayoutState.h — Persistent layout flags and dock state.
#pragma once

#include <algorithm>
#include <string>
#include "../include/ProjectConfig.h"

struct EditorLayoutState {
    int bottomTrayActiveIdx = 0;
    bool bottomTrayVisible = true;
    bool focusViewportMode = false;

    void SaveToIni(LightningEngine::IniFile& ini, int activeBottomTab) const {
        const int clampedActive = std::max(0, activeBottomTab);
        ini.Set("Containers", "BottomTrayActive", std::to_string(clampedActive));
        ini.Set("Containers", "BottomTrayVisible", bottomTrayVisible ? "true" : "false");
        ini.Set("Containers", "FocusViewportMode", focusViewportMode ? "true" : "false");
    }

    void LoadFromIni(const LightningEngine::IniFile& ini) {
        bottomTrayActiveIdx = std::max(0, ini.GetInt("Containers", "BottomTrayActive", bottomTrayActiveIdx));
        bottomTrayVisible = ini.GetBool("Containers", "BottomTrayVisible", bottomTrayVisible);
        focusViewportMode = ini.GetBool("Containers", "FocusViewportMode", focusViewportMode);
    }
};
