// EditorContext.h — Shared state passed to all editor panels.
// Provides access to engine subsystems and cross-panel callbacks
// without creating circular dependencies.
#pragma once
#include <string>
#include <functional>
#include "../include/Renderer.h"
#include "../include/InputManager.h"
#include "../include/ProjectManager.h"
#include "../include/Level.h"
#include "../include/UndoStack.h"
#include "../include/gui/TitanUI.h"
#include "../include/gui/TitanStyle.h"
#include "../include/Node.h"
#include "tabs/EditorTabSystem.h"

namespace fs = std::filesystem;
using namespace LightningEngine;
using namespace Titan;

struct EditorContext
{
    // ── Core subsystems (non-owning references) ───────────────────────────
    Renderer&       renderer;
    InputManager&   inputManager;
    TitanUI&        ui;
    ProjectManager& pm;
    Level&          editorLevel;
    UndoStack&      undoStack;
    LightningEditor::EditorTabManager& tabManager;

    // ── Shared editor state ───────────────────────────────────────────────
    Node*&          selectedNode;
    std::string&    currentScenePath;
    bool&           isPlaying;

    // ── Shared textures ───────────────────────────────────────────────────
    Texture&        gridTex;

    // ── Layout (updated by EditorApp on resize) ───────────────────────────
    float&          kW;
    float&          kH;
    float           kHierW   = 230.f;
    float           kLeftW   = 230.f;
    float           kScriptH = 180.f;
    float           kTopH    = 102.f;    // kTitleH + kMenuH + kToolH

    // ── Cross-panel callbacks ─────────────────────────────────────────────
    std::function<void()>                         onHierarchyChanged;
    std::function<void()>                         onInspectorRefresh;
    std::function<void()>                         onContentBrowserRefresh;
    std::function<void(const std::string&)>       noteChange;
    std::function<std::string(const std::string&)> resolveFilePath;
    std::function<void(const std::string&)>       openAssetTab;
    std::function<void(float)>                    adjustFontScale;
    std::function<void(RichText*)>                applyTextPrefs;

    // ── Helpers ───────────────────────────────────────────────────────────
    fs::path contentRootDir() const
    {
        return pm.isOpen ? fs::path(pm.project.rootPath) : fs::path();
    }

    fs::path makeUniquePath(const fs::path& dir, const std::string& stem, const std::string& ext) const
    {
        fs::path candidate = dir / (stem + ext);
        if (!fs::exists(candidate)) return candidate;
        for (int i = 1; i < 1000; ++i) {
            candidate = dir / (stem + "_" + std::to_string(i) + ext);
            if (!fs::exists(candidate)) return candidate;
        }
        return dir / (stem + "_x" + ext);
    }
};
