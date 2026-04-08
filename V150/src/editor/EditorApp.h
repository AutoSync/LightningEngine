// EditorApp.h — Lightning Engine Editor v0.4
//
// Architecture:
//   State::Splash  — project picker (no project open)
//   State::Editor  — full editor (project loaded)
//
// Integration:
//   FILE menu      ↔  ProjectManager  (create / open / save scene / close)
//   Hierarchy      ↔  EditorLevel (real Level/Node tree)
//   Inspector      ↔  real Node* transform / name / active
//   Viewport       ↔  editorLevel.Render()  +  editorLevel.Update(dt) when playing
//   Content Browser↔  std::filesystem scan of pm.AssetsDir()
//   Play           ↔  editorLevel.Update(dt) each frame (scripts run via NucleoScript)
//
// ESC = quit
#pragma once
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <cctype>
#include <limits>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include "../include/GameInstance.h"
#include "../include/Texture.h"
#include "../include/ProjectManager.h"
#include "../include/Nucleo.h"
#include "../include/Logger.h"
#include "../include/UndoStack.h"
#include "../include/NativeDialog.h"
#include "../include/GamePreviewWindow.h"
#include "../include/Equinox.h"
#include "../include/gui/TitanUI.h"
#include "../include/gui/TitanStyle.h"
#include "../include/gui/widgets/TextureViewerWidget.h"
#include "../include/components/SpriteRenderer.h"
#include "../include/EditorElementRegistry.h"
#include "tabs/EditorTabSystem.h"
#include "tabs/EditorDocumentContent.h"
#include "tabs/EditorDocumentWorkspaceRenderer.h"

namespace fs = std::filesystem;
using namespace Titan;
using namespace LightningEngine;

// ─────────────────────────────────────────────────────────────────────────────
// EditorApp
// ─────────────────────────────────────────────────────────────────────────────
class EditorApp : public GameInstance
{
private:

    // ── Concrete Level for the editor scene ──────────────────────────────
    class EditorLevel : public Level {
    public:
        void Initialize() override {}
        void Shutdown()   override {}
    };

    // ── Editor state ──────────────────────────────────────────────────────
    enum class State { Splash, Editor };

    TitanUI        ui;
    State          state        = State::Splash;
    ProjectManager pm;
    EditorLevel    editorLevel;
    UndoStack      undoStack;
    Node*          selectedNode = nullptr;   // real Node*, nullptr = none
    std::string    currentScenePath;         // relative to project root

    // ── Layout constants ──────────────────────────────────────────────────
    static constexpr float kTitleH    = 46.f;
    static constexpr float kMenuH     = 26.f;
    static constexpr float kToolH     = 30.f;
    static constexpr float kFootH     = 24.f;
    static constexpr float kLeftW     = 230.f;
    static constexpr float kHierW     = 230.f;
    static constexpr float kCamH      = 118.f;
    static constexpr float kScriptH   = 180.f;
    static constexpr float kConsoleW  = 420.f;
    static constexpr float kTopH      = kTitleH + kMenuH + kToolH;
    static constexpr float kLogoW     = 44.f;
    static constexpr float kProjNameW = 115.f;

    float kW = 1280.f, kH = 720.f;
    float kMainH      = 0.f;
    float kHierSplitH = 0.f;
    float kInspH      = 0.f;
    float kVpW        = 0.f;
    float kVpH        = 0.f;

    void computeLayout()
    {
        kMainH      = kH - kTopH - kScriptH - kFootH;
        kHierSplitH = kMainH * 0.55f;
        kInspH      = kMainH - kHierSplitH - 4.f;
        kVpW        = kW - kLeftW - 8.f - kHierW;
        kVpH        = kMainH;
    }

    // ── UI panel pointers ─────────────────────────────────────────────────
    Panel*       pSplash         = nullptr;
    Panel*       pNewProjModal   = nullptr;
    Panel*       pOpenProjModal  = nullptr;
    TextField*   tfNewName       = nullptr;
    TextField*   tfNewPath       = nullptr;
    TextField*   tfOpenPath      = nullptr;
    Label*       lblNewStatus    = nullptr;
    Label*       lblOpenStatus   = nullptr;
    TabStrip*    pTabStrip       = nullptr;
    MenuBar*     pMenuBar        = nullptr;
    Toolbar*     pToolbar        = nullptr;
    DockSpace*   pDockSpace      = nullptr;
    DockNode*    pBottomTrayNode = nullptr;
    Panel*       pCamWidget      = nullptr;
    Panel*       pContentBrow    = nullptr;
    Panel*       pHierarchy      = nullptr;
    Panel*       pInspector      = nullptr;
    float        inspLastW       = 0.f;
    Viewport2D*  pViewport       = nullptr;
    TreeView*    pHierTree       = nullptr;
    TreeView*    pCBTree         = nullptr;
    ScrollView*  pCBScroll       = nullptr;
    Grid*        pCBGrid         = nullptr;
    Panel*       pScriptPanel    = nullptr;
    RichText*    pScriptEdit     = nullptr;
    DockSpace*   pScriptDock     = nullptr;
    Panel*       pDocumentFilesPanel = nullptr;
    Panel*       pDocumentOutlinePanel = nullptr;
    Panel*       pDocumentEditorPanel = nullptr;
    RichText*    pScriptDockEdit = nullptr;
    Panel*       pConsolePanel   = nullptr;
    ScrollView*  pConsoleSV      = nullptr;
    Panel*       pFooter         = nullptr;
    Label*       lblFooterLeft   = nullptr;
    Label*       lblFooterRight  = nullptr;
    bool         consoleVisible  = true;
    int          newProjTemplate = 0;   // 0=Empty 1=2D 2=3D
    ContextMenu* hierMenu        = nullptr;
    ContextMenu* cbMenu          = nullptr;
    Panel*       pCBRenameModal  = nullptr;
    TextField*   tfCBRename      = nullptr;
    Label*       lblCBRenameStat = nullptr;
    Panel*       pCBPropsModal   = nullptr;
    Panel*       pPluginWizardModal = nullptr;
    TextField*   tfPluginName       = nullptr;
    TextField*   tfPluginId         = nullptr;
    TextField*   tfPluginVersion    = nullptr;
    TextField*   tfPluginCategory   = nullptr;
    TextField*   tfPluginSubcat     = nullptr;
    Label*       lblPluginWizardStat = nullptr;
    int          pluginScopeIdx     = 0; // 0=Project, 1=Global
    int          pluginTypeIdx      = 0; // 0=C++, 1=C#, 2=Ignite

    std::string  cbCurrentDir;
    std::string  cbSelectedPath;
    bool         cbSelectedIsDir = false;
    std::string  cbLastClickPath;
    Uint64       cbLastClickMs   = 0;
    std::string  cbRenameTargetPath;
    std::string  cbRenameTargetExt;
    std::vector<std::pair<TreeNode*, std::string>> cbTreeNodePaths;
    int          cbTypeFilter    = 0;   // 0=All
    int          cbViewMode      = 0;   // 0=Grid, 1=List
    float        cbLastPanelW    = 0.f;
    float        cbLastPanelH    = 0.f;

    Texture logoTex;
    Texture projectIconTex;
    Texture splashTex;
    Texture gridTex;

    // Content Browser icon atlas (loaded once, non-owning refs used by buttons)
    Texture cbIconDirThumb;
    Texture cbIconDirCppThumb;
    Texture cbIconDirPluginsThumb;
    Texture cbIconFolder;
    Texture cbIconDefault;
    Texture cbIconTexture;
    Texture cbIconScript;
    Texture cbIconScene;
    Texture cbIconShader;
    Texture cbIconConfig;
    Texture cbIconMaterial;
    Texture cbIconStaticMesh;
    Texture cbIconParticles;

    // Texture editor — loaded on demand when a Texture tab is active
    Texture     texEditorTex;
    std::string texEditorPath;   // path of the currently loaded texEditorTex

    Label* lblFps   = nullptr;
    float  fpsTimer = 0.f;
    int    fpsCount = 0;
    float  curFps   = 0.f;

    bool                isPlaying    = false;
    GamePreviewWindow   gamePreview;

    // Drag & Drop from Content Browser
    bool        cbDragging    = false;   // is a file being dragged?
    std::string cbDragFile;              // absolute path being dragged
    std::string cbDragExt;              // file extension

    // Gizmo drag state
    enum class GizmoAxis { None, X, Y, XY };
    GizmoAxis gizmoAxis       = GizmoAxis::None;
    float     gizmoDragMX0    = 0.f;  // mouse pos when drag started
    float     gizmoDragMY0    = 0.f;
    float     gizmoDragNX0    = 0.f;  // node pos when drag started
    float     gizmoDragNY0    = 0.f;
    float     vpAX            = 0.f;  // viewport screen offset (updated each render)
    float     vpAY            = 0.f;
    float     viewportCamX    = 0.f;
    float     viewportCamY    = 0.f;
    float     viewportZoom    = 1.f;
    float     viewportGridSize = 32.f;
    bool      viewportShowGrid = true;
    bool      viewportSnapToGrid = false;
    bool      viewportPanning = false;
    float     viewportPanStartMX = 0.f;
    float     viewportPanStartMY = 0.f;
    float     viewportPanCamX0 = 0.f;
    float     viewportPanCamY0 = 0.f;

    enum class EditorCursorMode { Default, Text, Pan };
    SDL_Cursor* cursorDefault = nullptr;
    SDL_Cursor* cursorText    = nullptr;
    SDL_Cursor* cursorPan     = nullptr;
    EditorCursorMode cursorMode = EditorCursorMode::Default;
    int       cacheBottomTrayActiveIdx = 0;
    std::string cacheLastChange = "Session start";
    bool      cacheDirty = false;
    bool      cachePersistenceSuspended = false;
    float     codeEditorFontScale = 1.0f;

    LightningEditor::EditorTabManager tabManager;

    std::string resolveProjectFilePath(const std::string& rawPath) const
    {
        if (rawPath.empty()) return {};

        fs::path input(rawPath);
        if (input.is_absolute()) return input.string();
        if (!pm.isOpen) return rawPath;

        const fs::path root(pm.project.rootPath);
        const fs::path candidates[] = {
            root / input,
            fs::path(pm.ScriptsDir()) / input,
            fs::path(pm.AssetsDir()) / input,
            fs::path(pm.ScenesDir()) / input,
        };

        for (const auto& candidate : candidates) {
            if (fs::exists(candidate)) return candidate.string();
        }

        return (root / input).string();
    }

    fs::path contentRootDir() const
    {
        return pm.isOpen ? fs::path(pm.project.rootPath) : fs::path();
    }

    bool ensureCBDirValid()
    {
        if (!pm.isOpen) return false;
        fs::path root = contentRootDir();
        if (root.empty()) return false;

        std::error_code ec;
        if (cbCurrentDir.empty()) cbCurrentDir = root.string();

        fs::path cur(cbCurrentDir);
        if (!fs::exists(cur, ec) || !fs::is_directory(cur, ec)) {
            cbCurrentDir = root.string();
            return true;
        }

        fs::path rel = fs::relative(cur, root, ec);
        if (ec || rel.empty() || rel.string().rfind("..", 0) == 0) {
            cbCurrentDir = root.string();
            return true;
        }
        return true;
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

    fs::path editorCacheDir() const
    {
        char* appdata = nullptr;
        size_t appdataLen = 0;
        std::string base = ".";
        if (_dupenv_s(&appdata, &appdataLen, "APPDATA") == 0 && appdata) {
            base = appdata;
            std::free(appdata);
        }
        return fs::path(base) / "LightningEngine";
    }

    fs::path cacheFilePath() const
    {
        return editorCacheDir() / "editor_cache.ini";
    }

    fs::path legacyCacheFilePath() const
    {
        if (pm.isOpen) return fs::path(pm.project.rootPath) / "editor_cache.ini";
        return fs::current_path() / ".lightning_editor_cache.ini";
    }

    void noteEngineChange(const std::string& message)
    {
        if (cachePersistenceSuspended) return;
        cacheLastChange = message + " @" + std::to_string((unsigned long long)SDL_GetTicks());
        cacheDirty = true;
        saveEditorCache();
    }

    void saveEditorCache()
    {
        fs::path path = cacheFilePath();
        std::error_code ec;
        if (path.has_parent_path()) fs::create_directories(path.parent_path(), ec);

        IniFile ini;

        SDL_Window* win = renderer.GetWindow();
        if (win) {
            int ww = 0, wh = 0;
            SDL_GetWindowSize(win, &ww, &wh);
            const Uint64 flags = SDL_GetWindowFlags(win);
            ini.Set("Window", "Width", std::to_string(ww));
            ini.Set("Window", "Height", std::to_string(wh));
            ini.Set("Window", "Fullscreen", (flags & SDL_WINDOW_FULLSCREEN) ? "true" : "false");
            ini.Set("Window", "Maximized", (flags & SDL_WINDOW_MAXIMIZED) ? "true" : "false");
        }

        ini.Set("Viewport", "CamX", std::to_string(viewportCamX));
        ini.Set("Viewport", "CamY", std::to_string(viewportCamY));
        ini.Set("Viewport", "Zoom", std::to_string(viewportZoom));
        ini.Set("Viewport", "ShowGrid", viewportShowGrid ? "true" : "false");
        ini.Set("Viewport", "SnapToGrid", viewportSnapToGrid ? "true" : "false");

        ini.Set("ContentBrowser", "CurrentDir", cbCurrentDir);
        ini.Set("ContentBrowser", "SelectedPath", cbSelectedPath);
        ini.Set("ContentBrowser", "TypeFilter", std::to_string(cbTypeFilter));
        ini.Set("ContentBrowser", "ViewMode", std::to_string(cbViewMode));

        ini.Set("Containers", "BottomTrayActive", std::to_string(pBottomTrayNode ? pBottomTrayNode->activeIdx : cacheBottomTrayActiveIdx));
        ini.Set("Containers", "HierarchyX", std::to_string(pHierarchy ? pHierarchy->x : 0.f));
        ini.Set("Containers", "HierarchyY", std::to_string(pHierarchy ? pHierarchy->y : 0.f));
        ini.Set("Containers", "HierarchyW", std::to_string(pHierarchy ? pHierarchy->w : 0.f));
        ini.Set("Containers", "HierarchyH", std::to_string(pHierarchy ? pHierarchy->h : 0.f));
        ini.Set("Containers", "InspectorX", std::to_string(pInspector ? pInspector->x : 0.f));
        ini.Set("Containers", "InspectorY", std::to_string(pInspector ? pInspector->y : 0.f));
        ini.Set("Containers", "InspectorW", std::to_string(pInspector ? pInspector->w : 0.f));
        ini.Set("Containers", "InspectorH", std::to_string(pInspector ? pInspector->h : 0.f));
        ini.Set("Containers", "ViewportX", std::to_string(pViewport ? pViewport->x : 0.f));
        ini.Set("Containers", "ViewportY", std::to_string(pViewport ? pViewport->y : 0.f));
        ini.Set("Containers", "ViewportW", std::to_string(pViewport ? pViewport->w : 0.f));
        ini.Set("Containers", "ViewportH", std::to_string(pViewport ? pViewport->h : 0.f));

        ini.Set("Changes", "LastEngineChange", cacheLastChange);
        ini.Set("CodeEditor", "FontScale", std::to_string(codeEditorFontScale));

        if (ini.Save(path.string())) {
            cacheDirty = false;
        } else {
            Logger::LogWarning("[Editor] Failed to save editor cache: " + path.string());
        }
    }

    bool loadEditorCacheFrom(const fs::path& path)
    {
        IniFile ini;
        if (!ini.Load(path.string())) return false;

        auto readFloat = [&](const char* sec, const char* key, float defVal) {
            try { return std::stof(ini.Get(sec, key, std::to_string(defVal))); }
            catch (...) { return defVal; }
        };

        SDL_Window* win = renderer.GetWindow();
        if (win) {
            int ww = ini.GetInt("Window", "Width", (int)kW);
            int wh = ini.GetInt("Window", "Height", (int)kH);
            if (ww > 320 && wh > 240) {
                SDL_SetWindowSize(win, ww, wh);
                kW = (float)ww;
                kH = (float)wh;
            }
            if (ini.GetBool("Window", "Fullscreen", false)) SDL_SetWindowFullscreen(win, true);
            if (ini.GetBool("Window", "Maximized", false)) SDL_MaximizeWindow(win);
        }

        viewportCamX = readFloat("Viewport", "CamX", viewportCamX);
        viewportCamY = readFloat("Viewport", "CamY", viewportCamY);
        viewportZoom = std::clamp(readFloat("Viewport", "Zoom", viewportZoom), 0.2f, 8.0f);
        viewportShowGrid = ini.GetBool("Viewport", "ShowGrid", viewportShowGrid);
        viewportSnapToGrid = ini.GetBool("Viewport", "SnapToGrid", viewportSnapToGrid);

        cbCurrentDir = ini.Get("ContentBrowser", "CurrentDir", cbCurrentDir);
        cbSelectedPath = ini.Get("ContentBrowser", "SelectedPath", cbSelectedPath);
        cbTypeFilter = ini.GetInt("ContentBrowser", "TypeFilter", cbTypeFilter);
        cbViewMode = ini.GetInt("ContentBrowser", "ViewMode", cbViewMode);

        cacheBottomTrayActiveIdx = std::max(0, ini.GetInt("Containers", "BottomTrayActive", cacheBottomTrayActiveIdx));
        cacheLastChange = ini.Get("Changes", "LastEngineChange", cacheLastChange);
        codeEditorFontScale = std::clamp(readFloat("CodeEditor", "FontScale", codeEditorFontScale), 0.75f, 2.5f);
        return true;
    }

    void loadEditorCache()
    {
        cacheDirty = false;
        if (loadEditorCacheFrom(cacheFilePath())) return;
        if (loadEditorCacheFrom(legacyCacheFilePath())) saveEditorCache();
    }

    void applyEditorTextPreferences(RichText* editor)
    {
        if (!editor) return;
        editor->SetFontScale(codeEditorFontScale);
        editor->showLineNums = true;
        editor->onFontScaleChanged = [this](float scale) {
            codeEditorFontScale = std::clamp(scale, 0.75f, 2.5f);
            noteEngineChange("Code editor font scale changed");
        };
    }

    void adjustCodeEditorFontScale(float delta)
    {
        float next = std::clamp(codeEditorFontScale + delta, 0.75f, 2.5f);
        if (std::fabs(next - codeEditorFontScale) < 0.001f) return;
        codeEditorFontScale = next;

        applyEditorTextPreferences(pScriptEdit);
        applyEditorTextPreferences(pScriptDockEdit);

        noteEngineChange("Code editor font scale changed");
    }

    std::string normalizePluginId(const std::string& source) const
    {
        std::string out;
        out.reserve(source.size());
        bool lastDash = false;

        for (unsigned char ch : source) {
            if (std::isalnum(ch)) {
                out.push_back((char)std::tolower(ch));
                lastDash = false;
                continue;
            }

            if (ch == '.' || ch == '_' || ch == '-') {
                if (out.empty() || lastDash) continue;
                out.push_back('-');
                lastDash = true;
                continue;
            }

            if (std::isspace(ch)) {
                if (out.empty() || lastDash) continue;
                out.push_back('-');
                lastDash = true;
            }
        }

        while (!out.empty() && out.back() == '-') out.pop_back();
        if (out.empty()) out = "new-plugin";
        return out;
    }

    fs::path pluginBaseDirForScope(int scopeIdx) const
    {
        if (scopeIdx == 1) {
            return fs::current_path() / "plugins" / "global";
        }
        if (!pm.isOpen) return fs::path();
        return fs::path(pm.project.rootPath) / "plugins";
    }

    void addPluginToProject()
    {
        if (!pm.isOpen) {
            Logger::LogWarning("[Editor] Add Plugin: open a project first.");
            return;
        }

        std::string picked = NativeDialog::PickFolderSDL(renderer.GetWindow(), "Select Plugin Folder");
        if (picked.empty()) return;

        std::error_code ec;
        fs::path src(picked);
        if (!fs::exists(src, ec) || !fs::is_directory(src, ec)) {
            Logger::LogWarning("[Editor] Add Plugin: invalid folder.");
            return;
        }

        fs::path dstRoot = fs::path(pm.project.rootPath) / "plugins";
        fs::create_directories(dstRoot, ec);
        if (ec) {
            Logger::LogWarning("[Editor] Add Plugin: cannot create plugin root.");
            return;
        }

        fs::path dst = makeUniquePath(dstRoot, src.filename().string(), "");
        fs::copy(src, dst, fs::copy_options::recursive, ec);
        if (ec) {
            Logger::LogWarning("[Editor] Add Plugin failed: " + ec.message());
            return;
        }

        Logger::LogInfo("[Editor] Plugin added: " + dst.filename().string());
        noteEngineChange("Plugin added: " + dst.filename().string());
        if (pm.isOpen) refreshContentBrowser();
    }

    void createPluginWizardProject()
    {
        if (!tfPluginName || !tfPluginVersion || !tfPluginCategory || !tfPluginSubcat) return;

        std::string name = tfPluginName->text;
        std::string rawId = tfPluginId ? tfPluginId->text : "";
        if (name.empty()) {
            if (lblPluginWizardStat) lblPluginWizardStat->SetText("Plugin name is required.");
            return;
        }

        std::string pluginId = normalizePluginId(rawId.empty() ? name : rawId);
        fs::path base = pluginBaseDirForScope(pluginScopeIdx);
        if (base.empty()) {
            if (lblPluginWizardStat) lblPluginWizardStat->SetText("Open a project for project-scope plugin.");
            return;
        }

        std::error_code ec;
        fs::create_directories(base, ec);
        if (ec) {
            if (lblPluginWizardStat) lblPluginWizardStat->SetText("Failed to create plugin base folder.");
            return;
        }

        fs::path pluginDir = base / pluginId;
        if (fs::exists(pluginDir, ec)) {
            pluginDir = makeUniquePath(base, pluginId, "");
            pluginId = pluginDir.filename().string();
        }

        fs::create_directories(pluginDir / "src", ec);
        if (ec) {
            if (lblPluginWizardStat) lblPluginWizardStat->SetText("Failed to create plugin folder.");
            return;
        }

        const std::string version = tfPluginVersion->text.empty() ? "0.1.0" : tfPluginVersion->text;
        const std::string category = tfPluginCategory->text.empty() ? "General" : tfPluginCategory->text;
        const std::string subcat = tfPluginSubcat->text.empty() ? "Core" : tfPluginSubcat->text;
        const bool globalScope = (pluginScopeIdx == 1);

        std::ofstream manifest((pluginDir / "plugin.yaml").string(), std::ios::binary | std::ios::trunc);
        manifest
            << "id: " << pluginId << "\n"
            << "name: \"" << name << "\"\n"
            << "version: " << version << "\n"
            << "scope: " << (globalScope ? "global" : "project") << "\n"
            << "category: \"" << category << "\"\n"
            << "subcategory: \"" << subcat << "\"\n"
            << "entrypoints:\n";

        if (pluginTypeIdx == 0) manifest << "  native: src/PluginMain.h\n";
        else if (pluginTypeIdx == 1) manifest << "  csharp: src/PluginMain.cs\n";
        else manifest << "  ignite: src/PluginMain.ignite\n";

        manifest << "permissions:\n"
                 << "  - editor-ui\n"
                 << "enabledByDefault: true\n";
        manifest.close();

        std::ofstream readme((pluginDir / "README.md").string(), std::ios::binary | std::ios::trunc);
        readme << "# " << name << "\n\n"
               << "Plugin generated by Lightning Engine Plugin Wizard.\n\n"
               << "- id: " << pluginId << "\n"
               << "- scope: " << (globalScope ? "global" : "project") << "\n"
               << "- category: " << category << " / " << subcat << "\n";
        readme.close();

        if (pluginTypeIdx == 0) {
            std::ofstream cpp((pluginDir / "src" / "PluginMain.h").string(), std::ios::binary | std::ios::trunc);
            cpp << "#pragma once\n"
                << "#include \"../../../../src/include/PluginContracts.h\"\n\n"
                << "class " << normalizePluginId(name) << "Plugin : public LightningEngine::IEnginePlugin {\n"
                << "public:\n"
                << "    const LightningEngine::PluginManifest& Manifest() const override { return manifest; }\n"
                << "    bool OnLoad() override { return true; }\n"
                << "    void OnRegister() override {}\n"
                << "    void OnActivate() override {}\n"
                << "    void OnDeactivate() override {}\n"
                << "    void OnUnload() override {}\n"
                << "private:\n"
                << "    LightningEngine::PluginManifest manifest{\n"
                << "        \"" << pluginId << "\", \"" << name << "\", \"" << version << "\"\n"
                << "    };\n"
                << "};\n";
            cpp.close();
        } else if (pluginTypeIdx == 1) {
            std::ofstream cs((pluginDir / "src" / "PluginMain.cs").string(), std::ios::binary | std::ios::trunc);
            cs << "namespace LightningPlugins {\n"
               << "    public class PluginMain {\n"
               << "        public string Id => \"" << pluginId << "\";\n"
               << "        public void OnLoad() {}\n"
               << "        public void OnActivate() {}\n"
               << "        public void OnDeactivate() {}\n"
               << "    }\n"
               << "}\n";
            cs.close();
        } else {
            std::ofstream ig((pluginDir / "src" / "PluginMain.ignite").string(), std::ios::binary | std::ios::trunc);
            ig << "plugin \"" << name << "\" {\n"
               << "    id: \"" << pluginId << "\"\n"
               << "    fn OnLoad() {}\n"
               << "    fn OnActivate() {}\n"
               << "    fn OnDeactivate() {}\n"
               << "}\n";
            ig.close();
        }

        if (lblPluginWizardStat) lblPluginWizardStat->SetText("Plugin created successfully.");
        Logger::LogInfo("[Editor] Plugin wizard created: " + pluginDir.string());
        noteEngineChange("Plugin wizard created: " + pluginId);
        refreshContentBrowser();
    }

    void openPluginWizardModal()
    {
        if (!pPluginWizardModal) {
            pPluginWizardModal = buildModalPanel(
                "Plugin Wizard",
                [this](Panel* p) {
                    float pad = gStyle.padding;
                    float lh = gStyle.lineH;
                    float mw = p->w;
                    float ty = gStyle.titleH + pad;

                    p->Add<Label>(pad, ty, "Plugin Name")->h = lh; ty += lh + 2.f;
                    tfPluginName = p->Add<TextField>(pad, ty, mw - pad * 2.f, lh + 2.f);
                    tfPluginName->SetText("MyPlugin");
                    ty += lh + 8.f;

                    p->Add<Label>(pad, ty, "Plugin ID (optional)")->h = lh; ty += lh + 2.f;
                    tfPluginId = p->Add<TextField>(pad, ty, mw - pad * 2.f, lh + 2.f);
                    tfPluginId->SetText("");
                    ty += lh + 8.f;

                    p->Add<Label>(pad, ty, "Version")->h = lh; ty += lh + 2.f;
                    tfPluginVersion = p->Add<TextField>(pad, ty, mw - pad * 2.f, lh + 2.f);
                    tfPluginVersion->SetText("0.1.0");
                    ty += lh + 8.f;

                    p->Add<Label>(pad, ty, "Category")->h = lh; ty += lh + 2.f;
                    tfPluginCategory = p->Add<TextField>(pad, ty, mw - pad * 2.f, lh + 2.f);
                    tfPluginCategory->SetText("Gameplay");
                    ty += lh + 8.f;

                    p->Add<Label>(pad, ty, "Subcategory")->h = lh; ty += lh + 2.f;
                    tfPluginSubcat = p->Add<TextField>(pad, ty, mw - pad * 2.f, lh + 2.f);
                    tfPluginSubcat->SetText("General");
                    ty += lh + 8.f;

                    p->Add<Label>(pad, ty, "Scope")->h = lh; ty += lh + 2.f;
                    auto* ddScope = p->Add<Dropdown>(pad, ty, mw - pad * 2.f, lh + 2.f, "Scope");
                    ddScope->AddItem("Project");
                    ddScope->AddItem("Global");
                    ddScope->SetSelected(pluginScopeIdx);
                    ddScope->onSelect = [this](int idx, const std::string&) { pluginScopeIdx = idx; };
                    ty += lh + 8.f;

                    p->Add<Label>(pad, ty, "Plugin Type")->h = lh; ty += lh + 2.f;
                    auto* ddType = p->Add<Dropdown>(pad, ty, mw - pad * 2.f, lh + 2.f, "Plugin Type");
                    ddType->AddItem("C++");
                    ddType->AddItem("C#");
                    ddType->AddItem("Ignite");
                    ddType->SetSelected(pluginTypeIdx);
                    ddType->onSelect = [this](int idx, const std::string&) { pluginTypeIdx = idx; };
                    ty += lh + 10.f;

                    lblPluginWizardStat = p->Add<Label>(pad, ty, "");
                    lblPluginWizardStat->h = lh;
                    lblPluginWizardStat->SetColor(220, 80, 80);
                    ty += lh + 8.f;

                    float bw = (mw - pad * 3.f) * 0.5f;
                    auto* btnCreate = p->Add<Button>(pad, ty, bw, lh + 4.f, "Create Plugin");
                    btnCreate->onClick = [this]{ createPluginWizardProject(); };
                    auto* btnCancel = p->Add<Button>(pad * 2.f + bw, ty, bw, lh + 4.f, "Cancel");
                    btnCancel->onClick = [this]{ if (pPluginWizardModal) pPluginWizardModal->visible = false; };
                },
                520.f
            );
        }

        if (pPluginWizardModal) {
            pPluginWizardModal->visible = true;
            if (lblPluginWizardStat) lblPluginWizardStat->SetText("");
            ui.BringToFront(pPluginWizardModal);
        }
    }

    bool isMouseOverViewport(float mx, float my) const
    {
        return pViewport &&
               mx >= vpAX && mx < vpAX + pViewport->w &&
               my >= vpAY && my < vpAY + pViewport->h;
    }

    Lightning::V2 viewportScreenToWorld(float sx, float sy) const
    {
        float zoom = std::max(viewportZoom, 0.001f);
        return {
            viewportCamX + (sx - vpAX) / zoom,
            viewportCamY + (sy - vpAY) / zoom
        };
    }

    Lightning::V2 viewportWorldToScreen(float wx, float wy) const
    {
        return {
            vpAX + (wx - viewportCamX) * viewportZoom,
            vpAY + (wy - viewportCamY) * viewportZoom
        };
    }

    float snapViewportValue(float value) const
    {
        if (!viewportSnapToGrid || viewportGridSize <= 0.f) return value;
        return std::round(value / viewportGridSize) * viewportGridSize;
    }

    Lightning::V2 snapViewportPoint(Lightning::V2 point) const
    {
        return { snapViewportValue(point.x), snapViewportValue(point.y) };
    }

    void focusViewportOn(const Lightning::V2& worldPoint)
    {
        if (!pViewport) return;
        float zoom = std::max(viewportZoom, 0.001f);
        viewportCamX = worldPoint.x - pViewport->w * 0.5f / zoom;
        viewportCamY = worldPoint.y - pViewport->h * 0.5f / zoom;
    }

    void frameViewportSelection()
    {
        if (selectedNode) {
            focusViewportOn(selectedNode->WorldPosition());
            return;
        }

        const auto& nodes = editorLevel.GetNodes();
        if (nodes.empty()) {
            viewportCamX = 0.f;
            viewportCamY = 0.f;
            return;
        }

        Lightning::V2 minP = nodes.front()->WorldPosition();
        Lightning::V2 maxP = minP;
        for (const auto& node : nodes) {
            Lightning::V2 wp = node->WorldPosition();
            minP.x = std::min(minP.x, wp.x);
            minP.y = std::min(minP.y, wp.y);
            maxP.x = std::max(maxP.x, wp.x);
            maxP.y = std::max(maxP.y, wp.y);
        }

        focusViewportOn({ (minP.x + maxP.x) * 0.5f, (minP.y + maxP.y) * 0.5f });
    }

    void resetViewportView()
    {
        viewportZoom = 1.f;
        frameViewportSelection();
    }

    void syncViewportToolbarState()
    {
        if (!pToolbar) return;
        for (auto& item : pToolbar->items) {
            if (item.label == "Grid") item.active = viewportShowGrid;
            if (item.label == "Snap") item.active = viewportSnapToGrid;
        }
    }

    void initializeEditorCursors()
    {
        cursorDefault = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
        cursorText    = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_TEXT);
        cursorPan     = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_MOVE);

        if (cursorDefault) SDL_SetCursor(cursorDefault);
        cursorMode = EditorCursorMode::Default;
    }

    void releaseEditorCursors()
    {
        if (cursorDefault) { SDL_DestroyCursor(cursorDefault); cursorDefault = nullptr; }
        if (cursorText)    { SDL_DestroyCursor(cursorText);    cursorText = nullptr; }
        if (cursorPan)     { SDL_DestroyCursor(cursorPan);     cursorPan = nullptr; }
    }

    void applyEditorCursor(EditorCursorMode mode)
    {
        if (mode == cursorMode) return;

        SDL_Cursor* target = cursorDefault;
        if (mode == EditorCursorMode::Text && cursorText) target = cursorText;
        if (mode == EditorCursorMode::Pan  && cursorPan)  target = cursorPan;

        if (target) SDL_SetCursor(target);
        cursorMode = mode;
    }

    void updateEditorCursor()
    {
        if (state != State::Editor) {
            applyEditorCursor(EditorCursorMode::Default);
            return;
        }

        const bool editingText =
            (pScriptEdit && pScriptEdit->isFocused) ||
            (pScriptDockEdit && pScriptDockEdit->isFocused);

        if (editingText) {
            applyEditorCursor(EditorCursorMode::Text);
            return;
        }

        if (viewportPanning) {
            applyEditorCursor(EditorCursorMode::Pan);
            return;
        }

        applyEditorCursor(EditorCursorMode::Default);
    }

    bool isCursorOverGizmo(float mx, float my) const
    {
        if (!selectedNode) return false;

        static constexpr float kArrow = 50.f;
        static constexpr float kHitR = 9.f;
        static constexpr float kCenterR = 7.f;

        Lightning::V2 wp = selectedNode->WorldPosition();
        Lightning::V2 screenPos = viewportWorldToScreen(wp.x, wp.y);
        auto dist = [](float ax, float ay, float bx, float by) {
            float dx = ax - bx;
            float dy = ay - by;
            return std::sqrt(dx * dx + dy * dy);
        };

        return dist(mx, my, screenPos.x, screenPos.y) < kCenterR ||
               dist(mx, my, screenPos.x + kArrow, screenPos.y) < kHitR ||
               dist(mx, my, screenPos.x, screenPos.y + kArrow) < kHitR;
    }

    void drawViewportGrid(Renderer& r, float vw, float vh)
    {
        if (!viewportShowGrid || viewportGridSize <= 0.f) return;

        float zoom = std::max(viewportZoom, 0.001f);
        float step = viewportGridSize;
        while (step * zoom < 14.f) step *= 2.f;

        float left = viewportCamX;
        float top = viewportCamY;
        float right = viewportCamX + vw / zoom;
        float bottom = viewportCamY + vh / zoom;
        float thickness = 1.f / zoom;

        float startX = std::floor(left / step) * step;
        float startY = std::floor(top / step) * step;

        for (float x = startX; x <= right + step; x += step) {
            bool axis = std::fabs(x) < 0.001f;
            r.SetDrawColor(axis ? 120 : 58, axis ? 120 : 58, axis ? 132 : 70, axis ? 180 : 90);
            r.DrawLine(x, top, x, bottom, thickness);
        }

        for (float y = startY; y <= bottom + step; y += step) {
            bool axis = std::fabs(y) < 0.001f;
            r.SetDrawColor(axis ? 120 : 58, axis ? 120 : 58, axis ? 132 : 70, axis ? 180 : 90);
            r.DrawLine(left, y, right, y, thickness);
        }
    }

    void pickViewportNodeRecursive(Node* node, float worldX, float worldY, float radiusWorld,
                                   Node*& bestNode, float& bestScore)
    {
        if (!node || !node->active) return;

        Lightning::V2 wp = node->WorldPosition();
        bool hit = false;
        float score = std::numeric_limits<float>::max();

        if (auto* sprite = node->GetComponent<SpriteRendererComponent>()) {
            float width = sprite->width > 0.f ? sprite->width :
                          (sprite->texture ? (float)sprite->texture->GetWidth() : 32.f);
            float height = sprite->height > 0.f ? sprite->height :
                           (sprite->texture ? (float)sprite->texture->GetHeight() : 32.f);
            width *= node->transform.Scale.x;
            height *= node->transform.Scale.y;
            float x = wp.x - width * sprite->pivot.x;
            float y = wp.y - height * sprite->pivot.y;
            if (worldX >= x && worldX <= x + width &&
                worldY >= y && worldY <= y + height) {
                hit = true;
                score = width * height;
            }
        }

        if (!hit) {
            float dx = worldX - wp.x;
            float dy = worldY - wp.y;
            float dist2 = dx * dx + dy * dy;
            if (dist2 <= radiusWorld * radiusWorld) {
                hit = true;
                score = dist2;
            }
        }

        if (hit && score <= bestScore) {
            bestNode = node;
            bestScore = score;
        }

        for (Node* child : node->GetChildren()) {
            pickViewportNodeRecursive(child, worldX, worldY, radiusWorld, bestNode, bestScore);
        }
    }

    Node* pickViewportNode(float mx, float my)
    {
        Lightning::V2 world = viewportScreenToWorld(mx, my);
        float radiusWorld = 12.f / std::max(viewportZoom, 0.001f);
        Node* bestNode = nullptr;
        float bestScore = std::numeric_limits<float>::max();

        const auto& nodes = editorLevel.GetNodes();
        for (auto it = nodes.rbegin(); it != nodes.rend(); ++it) {
            pickViewportNodeRecursive(it->get(), world.x, world.y, radiusWorld, bestNode, bestScore);
        }

        return bestNode;
    }

    void processViewportNavigation()
    {
        if (!pViewport || isPlaying) return;

        float mx = inputManager.GetMouseX();
        float my = inputManager.GetMouseY();
        bool overViewport = isMouseOverViewport(mx, my);

        if (overViewport) {
            bool changed = false;
            float scroll = inputManager.GetScrollWheelY();
            if (scroll != 0.f) {
                Lightning::V2 before = viewportScreenToWorld(mx, my);
                float zoomFactor = std::pow(1.12f, scroll);
                viewportZoom = std::clamp(viewportZoom * zoomFactor, 0.2f, 8.0f);
                viewportCamX = before.x - (mx - vpAX) / viewportZoom;
                viewportCamY = before.y - (my - vpAY) / viewportZoom;
                changed = true;
            }

            if (inputManager.IsKeyPressed(SDL_SCANCODE_G)) { viewportShowGrid = !viewportShowGrid; changed = true; }
            if (inputManager.IsKeyPressed(SDL_SCANCODE_S)) { viewportSnapToGrid = !viewportSnapToGrid; changed = true; }
            if (inputManager.IsKeyPressed(SDL_SCANCODE_F)) frameViewportSelection();
            if (inputManager.IsKeyPressed(SDL_SCANCODE_0)) resetViewportView();
            if (changed) noteEngineChange("Viewport settings changed");
            syncViewportToolbarState();
        }

        if (inputManager.IsMousePressed(2) && overViewport) {
            viewportPanning = true;
            viewportPanStartMX = mx;
            viewportPanStartMY = my;
            viewportPanCamX0 = viewportCamX;
            viewportPanCamY0 = viewportCamY;
        }

        if (viewportPanning) {
            if (inputManager.IsMouseDown(2)) {
                float zoom = std::max(viewportZoom, 0.001f);
                viewportCamX = viewportPanCamX0 - (mx - viewportPanStartMX) / zoom;
                viewportCamY = viewportPanCamY0 - (my - viewportPanStartMY) / zoom;
            } else {
                viewportPanning = false;
            }
        }
    }

    void processViewportSelection()
    {
        if (!pViewport || isPlaying || viewportPanning || gizmoAxis != GizmoAxis::None) return;
        if (!inputManager.IsMousePressed(1)) return;

        float mx = inputManager.GetMouseX();
        float my = inputManager.GetMouseY();
        if (!isMouseOverViewport(mx, my) || isCursorOverGizmo(mx, my)) return;

        Node* picked = pickViewportNode(mx, my);
        if (picked != selectedNode) {
            selectedNode = picked;
            rebuildHierarchyTree();
            refreshInspector();
        } else if (!picked) {
            refreshInspector();
        }
    }

    void openContentDirectory(const fs::path& dir)
    {
        if (!pm.isOpen) return;
        std::error_code ec;
        if (!fs::exists(dir, ec) || !fs::is_directory(dir, ec)) return;

        fs::path root = contentRootDir();
        fs::path rel  = fs::relative(dir, root, ec);
        if (ec || rel.string().rfind("..", 0) == 0) return;

        cbCurrentDir = dir.string();
        cbSelectedPath.clear();
        cbLastClickPath.clear();
        cbDragging = false;
        cbDragFile.clear();
        cbDragExt.clear();
        noteEngineChange("Content browser directory changed");
        refreshContentBrowser();
    }

    void handleCBEntryClick(const fs::path& absPath, bool isDirectory)
    {
        const std::string clicked = absPath.string();
        const Uint64 now = SDL_GetTicks();
        const bool isDouble = (clicked == cbLastClickPath) && (now - cbLastClickMs <= 350ULL);

        cbSelectedPath = clicked;
        cbSelectedIsDir = isDirectory;
        cbLastClickPath = clicked;
        cbLastClickMs = now;
        noteEngineChange("Content browser selection changed");

        if (!isDouble) return;

        if (isDirectory) openContentDirectory(absPath);
        else       openAssetContextTab(clicked);
    }

    void openSelectedCBProperties()
    {
        if (cbSelectedPath.empty()) {
            Logger::LogInfo("[Editor] Content Browser: selecione um arquivo para propriedades.");
            return;
        }

        fs::path p(cbSelectedPath);
        std::error_code ec;
        if (!fs::exists(p, ec)) {
            Logger::LogWarning("[Editor] Propriedades: arquivo nao existe.");
            return;
        }

        if (!pCBPropsModal) {
            pCBPropsModal = ui.AddRoot<Panel>(0.f, 0.f, 560.f, 220.f, "Propriedades do Arquivo", true);
            pCBPropsModal->zOrder = 250;
        }

        const bool isEquinoxAsset = !cbSelectedIsDir && EquinoxFileManager::IsEquinoxAssetPath(p.string());
        EquinoxDocumentInfo equinoxInfo;
        if (isEquinoxAsset) equinoxInfo = EquinoxFileManager::InspectDocument(p.string());

        pCBPropsModal->Clear();
        pCBPropsModal->visible = true;
        pCBPropsModal->w = 560.f;
        pCBPropsModal->h = isEquinoxAsset ? 320.f : 220.f;
        pCBPropsModal->x = (kW - pCBPropsModal->w) * 0.5f;
        pCBPropsModal->y = (kH - pCBPropsModal->h) * 0.5f;
        ui.BringToFront(pCBPropsModal);

        const float pad = gStyle.padding;
        const float lh = gStyle.lineH;
        float y = gStyle.titleH + pad;

        std::string type = cbSelectedIsDir ? "Directory" : p.extension().string();
        if (type.empty()) type = "(sem extensao)";
        std::string sizeText = "-";
        if (!cbSelectedIsDir) {
            auto sz = fs::file_size(p, ec);
            if (!ec) sizeText = std::to_string((unsigned long long)sz) + " bytes";
        }

        pCBPropsModal->Add<Label>(pad, y, ("Nome: " + p.filename().string()).c_str()); y += lh + 2.f;
        pCBPropsModal->Add<Label>(pad, y, ("Tipo: " + type).c_str()); y += lh + 2.f;
        pCBPropsModal->Add<Label>(pad, y, ("Tamanho: " + sizeText).c_str()); y += lh + 2.f;
        pCBPropsModal->Add<Label>(pad, y, ("Caminho: " + p.string()).c_str()); y += lh + 10.f;

        if (isEquinoxAsset) {
            pCBPropsModal->Add<Label>(pad, y, "Equinox")->SetColor(60, 150, 80); y += lh + 2.f;
            pCBPropsModal->Add<Label>(pad, y, ("Documento: " + equinoxInfo.displayName).c_str()); y += lh + 2.f;
            pCBPropsModal->Add<Label>(pad, y, ("Stage: " + equinoxInfo.stage).c_str()); y += lh + 2.f;
            pCBPropsModal->Add<Label>(pad, y, ("Output: " + equinoxInfo.output).c_str()); y += lh + 2.f;
            pCBPropsModal->Add<Label>(pad, y, ("Nodes detectados: " + std::to_string(equinoxInfo.nodeCount)).c_str()); y += lh + 2.f;
            pCBPropsModal->Add<Label>(pad, y, ("Parametros detectados: " + std::to_string(equinoxInfo.parameterCount)).c_str()); y += lh + 10.f;
        }

        auto* btnClose = pCBPropsModal->Add<Button>(pCBPropsModal->w - pad - 72.f,
                                                    pCBPropsModal->h - pad - (lh + 4.f),
                                                    72.f, lh + 4.f, "Fechar");
        btnClose->onClick = [this] {
            if (pCBPropsModal) pCBPropsModal->visible = false;
        };
    }

    void beginCBRenameSelected()
    {
        if (cbSelectedPath.empty()) return;

        fs::path p(cbSelectedPath);
        std::error_code ec;
        if (!fs::exists(p, ec)) return;

        const bool targetIsDir = fs::is_directory(p, ec);
        const bool targetIsFile = fs::is_regular_file(p, ec);
        if (!targetIsDir && !targetIsFile) return;

        cbRenameTargetPath = p.string();
        cbRenameTargetExt  = targetIsFile ? p.extension().string() : "";

        const char* modalTitle = targetIsDir ? "Renomear Diretorio" : "Renomear Arquivo";
        const char* inputLabel = targetIsDir
            ? "Novo nome do diretorio:"
            : "Novo nome (extensao preservada):";

        if (!pCBRenameModal) {
            pCBRenameModal = ui.AddRoot<Panel>(0.f, 0.f, 440.f, 150.f, modalTitle, true);
            pCBRenameModal->zOrder = 260;
        }

        pCBRenameModal->Clear();
        pCBRenameModal->title = modalTitle;
        pCBRenameModal->visible = true;
        pCBRenameModal->w = 440.f;
        pCBRenameModal->h = 150.f;
        pCBRenameModal->x = (kW - pCBRenameModal->w) * 0.5f;
        pCBRenameModal->y = (kH - pCBRenameModal->h) * 0.5f;
        ui.BringToFront(pCBRenameModal);

        const float pad = gStyle.padding;
        const float lh = gStyle.lineH;
        float y = gStyle.titleH + pad;

        pCBRenameModal->Add<Label>(pad, y, inputLabel);
        y += lh + 2.f;

        tfCBRename = pCBRenameModal->Add<TextField>(pad, y, pCBRenameModal->w - pad * 2.f, lh + 3.f);
        tfCBRename->SetText(p.stem().string());
        y += lh + 8.f;

        lblCBRenameStat = pCBRenameModal->Add<Label>(pad, y, "");
        lblCBRenameStat->SetColor(220, 80, 80);
        y += lh + 6.f;

        auto* btnOk = pCBRenameModal->Add<Button>(pad, y, 88.f, lh + 4.f, "Renomear");
        btnOk->onClick = [this] {
            if (!tfCBRename) return;
            std::string newStem = tfCBRename->text;
            while (!newStem.empty() && std::isspace((unsigned char)newStem.back())) newStem.pop_back();
            size_t i = 0;
            while (i < newStem.size() && std::isspace((unsigned char)newStem[i])) i++;
            if (i > 0) newStem = newStem.substr(i);

            if (newStem.empty()) {
                if (lblCBRenameStat) lblCBRenameStat->SetText("Nome invalido.");
                return;
            }

            fs::path oldP(cbRenameTargetPath);
            fs::path newP = oldP.parent_path() / (newStem + cbRenameTargetExt);
            std::error_code ec;
            if (newP == oldP) {
                if (pCBRenameModal) pCBRenameModal->visible = false;
                return;
            }
            if (fs::exists(newP, ec)) {
                if (lblCBRenameStat) lblCBRenameStat->SetText("Ja existe um arquivo com esse nome.");
                return;
            }

            fs::rename(oldP, newP, ec);
            if (ec) {
                if (lblCBRenameStat) lblCBRenameStat->SetText(("Falha ao renomear: " + ec.message()).c_str());
                return;
            }

            cbSelectedPath = newP.string();
            cbSelectedIsDir = fs::is_directory(newP, ec);
            cbLastClickPath.clear();
            if (pCBRenameModal) pCBRenameModal->visible = false;
            const bool renamedDir = cbRenameTargetExt.empty();
            noteEngineChange(std::string(renamedDir ? "Directory renamed: " : "File renamed: ") + newP.filename().string());
            refreshContentBrowser();
        };

        auto* btnCancel = pCBRenameModal->Add<Button>(pad + 92.f, y, 80.f, lh + 4.f, "Cancelar");
        btnCancel->onClick = [this] {
            if (pCBRenameModal) pCBRenameModal->visible = false;
        };
    }

    bool matchesCBTypeFilter(const fs::path& absFile) const
    {
        if (cbTypeFilter == 0) return true; // All

        std::string ext = LightningEditor::ToLowerCopy(absFile.extension().string());
        switch (cbTypeFilter) {
            case 1: // Images
                return ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga";
            case 2: // Scripts
                return ext == ".spark" || ext == ".cs";
            case 3: // Scenes/Prefabs
                return ext == ".lescene" || ext == ".lprefab" || ext == ".prefab";
            case 4: // Shaders
                return ext == ".vert" || ext == ".frag" || ext == ".spv" || ext == ".spark";
            case 5: // Config
                return ext == ".ini" || ext == ".json" || ext == ".yaml" || ext == ".yml" || ext == ".toml";
            default:
                return true;
        }
    }

    Texture* resolveCBDirectoryIcon(const fs::path& absDir)
    {
        std::string name = LightningEditor::ToLowerCopy(absDir.filename().string());
        if (name.find("script") != std::string::npos && cbIconDirCppThumb.IsValid())
            return &cbIconDirCppThumb;
        if (name.find("plugin") != std::string::npos && cbIconDirPluginsThumb.IsValid())
            return &cbIconDirPluginsThumb;
        if (cbIconDirThumb.IsValid()) return &cbIconDirThumb;
        if (cbIconFolder.IsValid()) return &cbIconFolder;
        if (cbIconDefault.IsValid()) return &cbIconDefault;
        return nullptr;
    }

    Texture* resolveCBFileIcon(const fs::path& absFile)
    {
        auto tab = LightningEditor::BuildAssetTab(absFile.string());
        switch (tab.kind) {
            case LightningEditor::EditorTabKind::Texture:
                if (cbIconTexture.IsValid()) return &cbIconTexture;
                break;
            case LightningEditor::EditorTabKind::Script:
                if (cbIconScript.IsValid()) return &cbIconScript;
                break;
            case LightningEditor::EditorTabKind::Scene:
            case LightningEditor::EditorTabKind::Prefab:
                if (cbIconScene.IsValid()) return &cbIconScene;
                break;
            case LightningEditor::EditorTabKind::Shader:
                if (cbIconShader.IsValid()) return &cbIconShader;
                break;
            case LightningEditor::EditorTabKind::Config:
                if (cbIconConfig.IsValid()) return &cbIconConfig;
                break;
            case LightningEditor::EditorTabKind::Material:
                if (cbIconMaterial.IsValid()) return &cbIconMaterial;
                break;
            case LightningEditor::EditorTabKind::StaticMesh:
            case LightningEditor::EditorTabKind::SkeletalMesh:
                if (cbIconStaticMesh.IsValid()) return &cbIconStaticMesh;
                break;
            case LightningEditor::EditorTabKind::Particle:
                if (cbIconParticles.IsValid()) return &cbIconParticles;
                break;
            default:
                break;
        }
        if (cbIconDefault.IsValid()) return &cbIconDefault;
        return nullptr;
    }

    std::string cbTreePathForNode(TreeNode* node) const
    {
        if (!node) return {};
        for (const auto& it : cbTreeNodePaths) {
            if (it.first == node) return it.second;
        }
        return {};
    }

    void buildCBTreeRecursive(TreeNode* parent, const fs::path& dir)
    {
        std::vector<fs::directory_entry> dirs;
        std::error_code ec;
        for (auto it = fs::directory_iterator(dir, ec); !ec && it != fs::directory_iterator(); ++it) {
            if (it->is_directory()) dirs.push_back(*it);
        }

        auto byName = [](const fs::directory_entry& a, const fs::directory_entry& b) {
            return LightningEditor::ToLowerCopy(a.path().filename().string()) <
                   LightningEditor::ToLowerCopy(b.path().filename().string());
        };
        std::sort(dirs.begin(), dirs.end(), byName);

        for (const auto& d : dirs) {
            TreeNode* tn = parent->AddChild(d.path().filename().string());
            tn->expanded = false;
            cbTreeNodePaths.push_back({ tn, d.path().string() });
            buildCBTreeRecursive(tn, d.path());
        }
    }

    void goContentParent()
    {
        if (!pm.isOpen || !ensureCBDirValid()) return;
        fs::path root = contentRootDir();
        fs::path cur(cbCurrentDir);
        if (cur == root) return;

        fs::path parent = cur.parent_path();
        if (parent.empty()) parent = root;
        openContentDirectory(parent);
    }

    void importFileToCurrentDir()
    {
        if (!pm.isOpen || !ensureCBDirValid()) return;
        std::string picked = NativeDialog::PickFileSDL(
            renderer.GetWindow(),
            "Import Asset",
            cbCurrentDir.c_str());
        if (picked.empty()) return;

        std::error_code ec;
        fs::path src(picked);
        if (!fs::exists(src, ec) || !fs::is_regular_file(src, ec)) {
            Logger::LogWarning("[Editor] Import failed: invalid file");
            return;
        }

        fs::path dstDir(cbCurrentDir);
        fs::path dst = makeUniquePath(dstDir, src.stem().string(), src.extension().string());
        fs::copy_file(src, dst, fs::copy_options::none, ec);
        if (ec) {
            Logger::LogWarning("[Editor] Import failed: " + ec.message());
            return;
        }

        Logger::LogInfo("[Editor] Imported: " + dst.filename().string());
        noteEngineChange("Asset imported: " + dst.filename().string());
        refreshContentBrowser();
        openAssetContextTab(dst.string());
    }

    void createFolderInCurrentDir()
    {
        if (!pm.isOpen || !ensureCBDirValid()) return;
        std::error_code ec;
        fs::path dir(cbCurrentDir);
        fs::path target = makeUniquePath(dir, "NewFolder", "");
        fs::create_directories(target, ec);
        if (ec) {
            Logger::LogWarning("[Editor] Failed to create folder: " + ec.message());
            return;
        }
        Logger::LogInfo("[Editor] Folder created: " + target.filename().string());
        noteEngineChange("Folder created: " + target.filename().string());
        refreshContentBrowser();
    }

    void createAssetInCurrentDir(const std::string& stem,
                                 const std::string& ext,
                                 const std::string& initial = "")
    {
        if (!pm.isOpen || !ensureCBDirValid()) return;
        fs::path dir(cbCurrentDir);
        fs::path target = makeUniquePath(dir, stem, ext);

        std::ofstream out(target.string(), std::ios::binary | std::ios::trunc);
        if (!out) {
            Logger::LogWarning("[Editor] Failed to create file: " + target.filename().string());
            return;
        }
        if (!initial.empty()) out << initial;
        out.close();

        Logger::LogInfo("[Editor] Created: " + target.filename().string());
        noteEngineChange("Asset created: " + target.filename().string());
        refreshContentBrowser();
        openAssetContextTab(target.string());
    }

    void createEquinoxAssetInCurrentDir(EquinoxAssetType type)
    {
        const std::string stem = EquinoxFileManager::DefaultStem(type);
        const std::string ext = EquinoxFileManager::DefaultExtension(type);
        if (ext.empty()) return;
        createAssetInCurrentDir(stem, ext, EquinoxFileManager::BuildDefaultDocument(type, stem));
    }

    fs::path equinoxWorkspaceDir() const
    {
        return fs::path(pm.AssetsDir()) / "equinox";
    }

    void openEquinoxWorkspace()
    {
        if (!pm.isOpen) return;

        std::error_code ec;
        fs::path dir = equinoxWorkspaceDir();
        fs::create_directories(dir, ec);
        if (ec) {
            Logger::LogWarning("[Editor] Equinox: failed to create workspace directory: " + ec.message());
            return;
        }

        openContentDirectory(dir);

        for (const auto& entry : fs::directory_iterator(dir, ec)) {
            if (ec) break;
            if (!entry.is_regular_file()) continue;
            if (!EquinoxFileManager::IsEquinoxAssetPath(entry.path().string())) continue;
            openAssetContextTab(entry.path().string());
            return;
        }

        createEquinoxAssetInCurrentDir(EquinoxAssetType::ShaderComposer);
    }

    void syncTabStripFromManager()
    {
        if (!pTabStrip) return;

        pTabStrip->tabs.clear();
        pTabStrip->contents.clear();
        pTabStrip->tabWidths.clear();
        pTabStrip->hoverTab = -1;
        pTabStrip->hoverClose = -1;
        pTabStrip->scrollFirst = 0;

        for (const auto& tab : tabManager.Tabs()) {
            pTabStrip->AddTab(tab.label.c_str(), tab.accent.r, tab.accent.g, tab.accent.b, tab.closable);
        }

        pTabStrip->activeTab = tabManager.ActiveIndex();
    }

    void resetTabsToSceneOnly()
    {
        tabManager.ResetToSceneOnly(currentScenePath);
        syncTabStripFromManager();
        switchLayout(false);
        refreshDocumentWorkspace();
    }

    void syncPrimarySceneTabLabel()
    {
        tabManager.SyncPrimarySceneTab(currentScenePath);
        syncTabStripFromManager();
    }

    void activateTabIndex(int idx)
    {
        tabManager.Activate(idx);
        if (pTabStrip) pTabStrip->activeTab = tabManager.ActiveIndex();
        switchLayout(tabManager.HasActiveDocument());
        refreshDocumentWorkspace();
    }

    void openAssetContextTab(const std::string& absPath)
    {
        tabManager.OpenAsset(absPath);
        syncTabStripFromManager();
        switchLayout(tabManager.HasActiveDocument());
        refreshDocumentWorkspace();
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Public lifecycle
    // ─────────────────────────────────────────────────────────────────────────
public:

    void Initialize() override
    {
        renderer.SetClearColor(20, 20, 26);
        ui.Init(renderer, "assets/fonts/Roboto-Regular.ttf", 13);
        logoTex   = renderer.LoadTexture("assets/icons/lightning.png");
        projectIconTex = renderer.LoadTexture("assets/icons/folder.png");
        splashTex = renderer.LoadTexture("assets/splash/splashscreen.png");
        gridTex   = buildGridTexture();

        cbIconDirThumb        = renderer.LoadTexture("assets/thumbnails/directories_thumb.png");
        cbIconDirCppThumb     = renderer.LoadTexture("assets/thumbnails/directories_cpp_thumb.png");
        cbIconDirPluginsThumb = renderer.LoadTexture("assets/thumbnails/directories_plugins_thumb.png");
        cbIconFolder          = renderer.LoadTexture("assets/icons/cb_folder.png");
        cbIconDefault         = renderer.LoadTexture("assets/icons/default.png");
        cbIconTexture         = renderer.LoadTexture("assets/icons/texture.png");
        cbIconScript          = renderer.LoadTexture("assets/icons/csharp.png");
        cbIconScene           = renderer.LoadTexture("assets/icons/scene.png");
        cbIconShader          = renderer.LoadTexture("assets/icons/shader.png");
        cbIconConfig          = renderer.LoadTexture("assets/icons/settings.png");
        cbIconMaterial        = renderer.LoadTexture("assets/icons/material.png");
        cbIconStaticMesh      = renderer.LoadTexture("assets/icons/static_mesh.png");
        cbIconParticles       = renderer.LoadTexture("assets/icons/particles.png");

        kW = (float)GetWidth();
        kH = (float)GetHeight();
        initializeEditorCursors();
        loadEditorCache();
        computeLayout();

        // Inject context into the live level (no nodes yet)
        editorLevel.SetContext(renderer, inputManager);
        editorLevel.Initialize();

        buildSplash();
    }

    void Shutdown() override
    {
        noteEngineChange("Editor shutdown");
        releaseEditorCursors();
        editorLevel.Shutdown();
        if (pm.isOpen) pm.Save();
        ui.Release();
        logoTex.Release();
        projectIconTex.Release();
        splashTex.Release();
        gridTex.Release();
        cbIconDirThumb.Release();
        cbIconDirCppThumb.Release();
        cbIconDirPluginsThumb.Release();
        cbIconFolder.Release();
        cbIconDefault.Release();
        cbIconTexture.Release();
        cbIconScript.Release();
        cbIconScene.Release();
        cbIconShader.Release();
        cbIconConfig.Release();
        cbIconMaterial.Release();
        cbIconStaticMesh.Release();
        cbIconParticles.Release();
        if (texEditorTex.IsValid()) { texEditorTex.Release(); texEditorPath.clear(); }
    }

    void Update(float dt) override
    {
        if (inputManager.IsKeyPressed(SDL_SCANCODE_ESCAPE)) Quit();

        // Window resize
        {
            int ww, wh;
            SDL_GetWindowSize(renderer.GetWindow(), &ww, &wh);
            if ((float)ww != kW || (float)wh != kH) {
                kW = (float)ww;
                kH = (float)wh;
                noteEngineChange("Window resized");
                computeLayout();
                if (state == State::Editor) {
                    if (pDockSpace) {
                        pDockSpace->Resize(0.f, kTopH, kW, kMainH + kScriptH);
                        if (pScriptDock)
                            pScriptDock->Resize(0.f, kTopH, kW, kMainH + kScriptH);
                        if (pTabStrip)    pTabStrip->w    = kW - kLogoW - kProjNameW;
                        if (pMenuBar)     pMenuBar->w     = kW;
                        if (pToolbar)     pToolbar->w     = kW;
                        if (pFooter) {
                            pFooter->x = 0.f;
                            pFooter->y = kH - kFootH;
                            pFooter->w = kW;
                            pFooter->h = kFootH;
                        }
                        if (lblFooterLeft) {
                            float ty = (kFootH - ui.font.GlyphH()) * 0.5f;
                            lblFooterLeft->x = gStyle.padding;
                            lblFooterLeft->y = ty;
                        }
                        if (lblFooterRight) {
                            const char* rLabel = pm.isOpen ? "Project Open" : "Ready";
                            lblFooterRight->SetText(rLabel);
                            float rW = ui.font.MeasureW(rLabel);
                            float ty = (kFootH - ui.font.GlyphH()) * 0.5f;
                            lblFooterRight->x = kW - rW - gStyle.padding;
                            lblFooterRight->y = ty;
                        }
                    } else {
                        rebuildEditorUI();
                    }
                } else {
                    rebuildSplash();
                }
            }
        }

        ui.ProcessInput(inputManager);
        updateEditorCursor();
        if (state == State::Editor) {
            if (inputManager.IsKeyPressed(SDL_SCANCODE_F2)) {
                beginCBRenameSelected();
            }

            // Ctrl+Z / Ctrl+Y — undo/redo
            bool ctrl = inputManager.IsKeyDown(SDL_SCANCODE_LCTRL) ||
                        inputManager.IsKeyDown(SDL_SCANCODE_RCTRL);
            if (ctrl && inputManager.IsKeyPressed(SDL_SCANCODE_Z)) undoStack.Undo();
            if (ctrl && inputManager.IsKeyPressed(SDL_SCANCODE_Y)) undoStack.Redo();
            if (ctrl && (inputManager.IsKeyPressed(SDL_SCANCODE_EQUALS) || inputManager.IsKeyPressed(SDL_SCANCODE_KP_PLUS)))
                adjustCodeEditorFontScale(+0.08f);
            if (ctrl && (inputManager.IsKeyPressed(SDL_SCANCODE_MINUS) || inputManager.IsKeyPressed(SDL_SCANCODE_KP_MINUS)))
                adjustCodeEditorFontScale(-0.08f);
        }

        if (state == State::Editor) {
            // Hierarchy right-click context menu
            if (inputManager.IsMousePressed(3)) {
                float mx = inputManager.GetMouseX();
                float my = inputManager.GetMouseY();
                bool overCB = pContentBrow && pContentBrow->Contains(mx, my, 0.f, 0.f);
                bool overHier = pHierarchy && pHierarchy->Contains(mx, my, 0.f, 0.f);
                if (overCB && cbMenu) {
                    rebuildContentBrowserContextMenu();
                    cbMenu->Open(mx, my);
                }
                else if (overHier && hierMenu) hierMenu->Open(mx, my);
            }

            // Play: tick real scene (scripts run here)
            if (isPlaying) editorLevel.Update(dt);

            // Game Preview window tick
            if (gamePreview.IsOpen()) gamePreview.Tick(dt);

            // Viewport camera/navigation
            processViewportNavigation();

            // Gizmo drag
            if (!isPlaying && selectedNode)
                processGizmoDrag();

            // Viewport click selection
            processViewportSelection();

            // Content Browser → Viewport Drag & Drop
            processCBDrop();

            // FPS
            fpsTimer += dt / 1000.f;
            fpsCount++;
            if (fpsTimer >= 1.f) {
                curFps   = (float)fpsCount / fpsTimer;
                fpsCount = 0; fpsTimer = 0.f;
                if (lblFps) {
                    char buf[32];
                    SDL_snprintf(buf, sizeof(buf), "%.1f fps", curFps);
                    lblFps->SetText(buf);
                }
            }

            // Console — refresh when new log entries arrive
            if (Logger::Dirty() && pConsoleSV) {
                refreshConsole();
                Logger::ClearDirty();
            }

            if (cacheDirty) {
                saveEditorCache();
            }
        }
    }

    void Render() override
    {
        renderer.Clear();
        if (state == State::Editor) renderChrome();
        ui.Render(renderer);

        // Drag & drop ghost tooltip
        if (cbDragging && !cbDragFile.empty()) {
            renderer.BeginScreenSpace();
            float mx = inputManager.GetMouseX();
            float my = inputManager.GetMouseY();
            std::string fname = fs::path(cbDragFile).filename().string();
            float tw = ui.font.MeasureW(fname.c_str()) + 10.f;
            renderer.SetDrawColor(30, 30, 45, 210);
            renderer.FillRect(mx + 12.f, my - 10.f, tw, 18.f);
            renderer.SetDrawColor(100, 140, 220, 200);
            renderer.DrawRect(mx + 12.f, my - 10.f, tw, 18.f);
            renderer.SetDrawColor(200, 200, 220);
            ui.font.DrawText(renderer, fname.c_str(), mx + 17.f, my - 10.f + (18.f - ui.font.GlyphH()) * 0.5f);
            renderer.EndScreenSpace();
        }

        renderer.Present();
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Splash / Project Picker
    // ─────────────────────────────────────────────────────────────────────────
private:

    void buildSplash()
    {
        ui.ClearRoots();
        clearEditorPtrs();

        static constexpr float kPanelW = 370.f;

        // ── Background: full-screen splash image ──────────────────────────
        pSplash = ui.AddRoot<Panel>(0.f, 0.f, kW, kH, "", false);
        pSplash->zOrder = 0;
        if (splashTex.IsValid()) {
            // Keep the splash centered in the usable area (screen minus right panel)
            // so the artwork doesn't look offset by the welcome panel overlay.
            float availW = std::max(1.f, kW - kPanelW);
            float texW   = (float)splashTex.GetWidth();
            float texH   = (float)splashTex.GetHeight();
            if (texW > 0.f && texH > 0.f) {
                float scale = std::min(availW / texW, kH / texH);
                float drawW = texW * scale;
                float drawH = texH * scale;
                float drawX = (availW - drawW) * 0.5f;
                float drawY = (kH - drawH) * 0.5f;
                pSplash->Add<Image>(drawX, drawY, drawW, drawH, &splashTex);
            }
        }

        // ── Right welcome panel ────────────────────────────────────────────
        auto* panel = ui.AddRoot<Panel>(kW - kPanelW, 0.f, kPanelW, kH, "", false);
        panel->zOrder = 5;

        float pad = gStyle.padding;
        float lh  = gStyle.lineH;
        float ty  = pad + 10.f;

        // ── Logo + title ───────────────────────────────────────────────────
        {
            float logoSz = 48.f;
            if (logoTex.IsValid())
                panel->Add<Image>(pad, ty, logoSz, logoSz, &logoTex);

            float textX = pad + logoSz + 10.f;
            auto* lblTitle = panel->Add<Label>(textX, ty + 8.f, "Lightning Engine");
            lblTitle->h = lh + 2.f;
            lblTitle->SetColor(gStyle.textBright.r, gStyle.textBright.g, gStyle.textBright.b);

            auto* lblVer = panel->Add<Label>(textX, ty + 8.f + lh + 3.f, "v0.15.0  \xe2\x80\x94  Editor");
            lblVer->h = lh;
            lblVer->SetColor(gStyle.textDim.r, gStyle.textDim.g, gStyle.textDim.b);

            ty += logoSz + pad + 8.f;
        }

        // ── Separator ──────────────────────────────────────────────────────
        {
            auto* sep = panel->Add<Separator>(kPanelW - pad * 2.f);
            sep->x = pad; sep->y = ty;
            ty += sep->h + 4.f;
        }

        // ── Action buttons ─────────────────────────────────────────────────
        {
            float bh = lh + 8.f;
            float bw = kPanelW - pad * 2.f;

            auto* btnNew = panel->Add<Button>(pad, ty, bw, bh, "  + New Project");
            btnNew->onClick = [this]{ openNewProjectModal(); };
            ty += bh + 5.f;

            auto* btnOpen = panel->Add<Button>(pad, ty, bw, bh, "  Open Project...");
            btnOpen->onClick = [this]{ openOpenProjectModal(); };
            ty += bh + 10.f;
        }

        // ── Separator ──────────────────────────────────────────────────────
        {
            auto* sep = panel->Add<Separator>(kPanelW - pad * 2.f);
            sep->x = pad; sep->y = ty;
            ty += sep->h + 4.f;
        }

        // ── Recent Projects ────────────────────────────────────────────────
        {
            auto* lblSection = panel->Add<Label>(pad, ty, "RECENT PROJECTS");
            lblSection->h = lh;
            lblSection->SetColor(gStyle.textDim.r, gStyle.textDim.g, gStyle.textDim.b);
            ty += lh + 8.f;

            auto recent = ProjectManager::GetRecentProjects();
            if (recent.empty()) {
                auto* lbl = panel->Add<Label>(pad + 4.f, ty, "No recent projects");
                lbl->h = lh;
                lbl->SetColor(gStyle.textDim.r, gStyle.textDim.g, gStyle.textDim.b);
            } else {
                for (const auto& projPath : recent) {
                    std::string name = fs::path(projPath).filename().string();
                    std::string displayPath = projPath;
                    if (displayPath.length() > 43)
                        displayPath = "..." + displayPath.substr(displayPath.length() - 40);

                    float entryH = lh * 2.f + 10.f;
                    std::string cap = projPath;

                    // Full-width clickable row (drawn first — behind labels)
                    auto* btn = panel->Add<Button>(0.f, ty, kPanelW, entryH, "");
                    btn->onClick = [this, cap]{ doOpenProject(cap); };

                    // Small project icon (higher legibility than the brand mark at tiny scale)
                    Texture* projIcon = projectIconTex.IsValid() ? &projectIconTex : &logoTex;
                    if (projIcon && projIcon->IsValid())
                        panel->Add<Image>(pad, ty + (entryH - 16.f) * 0.5f,
                                          16.f, 16.f, projIcon);

                    // Project name (bright)
                    auto* lblName = panel->Add<Label>(pad + 20.f, ty + 3.f, name.c_str());
                    lblName->h = lh;
                    lblName->SetColor(gStyle.textBright.r, gStyle.textBright.g, gStyle.textBright.b);

                    // Project path (dim)
                    auto* lblPath = panel->Add<Label>(pad + 20.f, ty + 3.f + lh + 1.f, displayPath.c_str());
                    lblPath->h = lh;
                    lblPath->SetColor(gStyle.textDim.r, gStyle.textDim.g, gStyle.textDim.b);

                    ty += entryH;

                    auto* sep = panel->Add<Separator>(kPanelW);
                    sep->x = 0.f; sep->y = ty;
                    ty += sep->h;
                }
            }
        }

        // ── Version footer ─────────────────────────────────────────────────
        {
            const char* footer = "Lightning Engine  \xc2\xa9 2025";
            auto* lbl = panel->Add<Label>(pad, kH - lh - pad * 2.f, footer);
            lbl->h = lh;
            lbl->SetColor(gStyle.textDim.r, gStyle.textDim.g, gStyle.textDim.b);
        }

        // ── New Project modal ─────────────────────────────────────────────
        pNewProjModal = buildModalPanel(
            "New Project",
            [this](Panel* p) {
                float pad = gStyle.padding, lh = gStyle.lineH;
                float mw  = p->w;
                float ty  = gStyle.titleH + pad;

                p->Add<Label>(pad, ty, "Project Name")->h = lh; ty += lh + 2.f;
                tfNewName = p->Add<TextField>(pad, ty, mw - pad * 2.f, lh + 2.f);
                tfNewName->SetText("MyGame");
                ty += lh + 10.f;

                // Folder Path row: TextField + Browse button
                p->Add<Label>(pad, ty, "Folder Path")->h = lh; ty += lh + 2.f;
                float browseBtnW = 28.f;
                float tfW = mw - pad * 3.f - browseBtnW;
                tfNewPath = p->Add<TextField>(pad, ty, tfW, lh + 2.f);
                tfNewPath->SetText("C:/Projects");
                auto* btnBrowseNew = p->Add<Button>(pad + tfW + pad, ty, browseBtnW, lh + 2.f, "...");
                btnBrowseNew->onClick = [this]{
                    std::string picked = NativeDialog::PickFolderSDL(
                        renderer.GetWindow(), "Select Project Folder");
                    if (!picked.empty() && tfNewPath)
                        tfNewPath->SetText(picked);
                };
                ty += lh + 10.f;

                // Template selector
                p->Add<Label>(pad, ty, "Template")->h = lh; ty += lh + 2.f;
                auto* dd = p->Add<Dropdown>(pad, ty, mw - pad * 2.f, lh + 2.f, "Select template...");
                dd->AddItem("Empty");
                dd->AddItem("2D Game");
                dd->AddItem("3D Game");
                dd->SetSelected(newProjTemplate);
                dd->onSelect = [this](int idx, const std::string&){ newProjTemplate = idx; };
                ty += lh + 10.f;

                lblNewStatus = p->Add<Label>(pad, ty, "");
                lblNewStatus->h = lh;
                lblNewStatus->SetColor(220, 80, 80);
                ty += lh + 10.f;

                float bw = (mw - pad * 3.f) * 0.5f;
                auto* btnOK = p->Add<Button>(pad, ty, bw, lh + 4.f, "Create");
                btnOK->onClick = [this]{
                    if (!tfNewName || !tfNewPath) return;
                    std::string n  = tfNewName->text;
                    std::string fp = tfNewPath->text + "/" + n;
                    doCreateProject(fp, n, newProjTemplate);
                };
                auto* btnCancel = p->Add<Button>(pad * 2.f + bw, ty, bw, lh + 4.f, "Cancel");
                btnCancel->onClick = [this]{ pNewProjModal->visible = false; };
            },
            270.f   // extra height for template row
        );

        // ── Open Project modal ────────────────────────────────────────────
        pOpenProjModal = buildModalPanel(
            "Open Project",
            [this](Panel* p) {
                float pad = gStyle.padding, lh = gStyle.lineH;
                float mw  = p->w;
                float ty  = gStyle.titleH + pad;

                // Path row: TextField + Browse button
                p->Add<Label>(pad, ty, "Project Folder or project.ini")->h = lh; ty += lh + 2.f;
                float browseBtnW = 28.f;
                float tfW = mw - pad * 3.f - browseBtnW;
                tfOpenPath = p->Add<TextField>(pad, ty, tfW, lh + 2.f);
                tfOpenPath->SetText("C:/Projects/MyGame");
                auto* btnBrowseOpen = p->Add<Button>(pad + tfW + pad, ty, browseBtnW, lh + 2.f, "...");
                btnBrowseOpen->onClick = [this]{
                    std::string picked = NativeDialog::PickFolderSDL(
                        renderer.GetWindow(), "Select Project Folder");
                    if (!picked.empty() && tfOpenPath)
                        tfOpenPath->SetText(picked);
                };
                ty += lh + 10.f;

                lblOpenStatus = p->Add<Label>(pad, ty, "");
                lblOpenStatus->h = lh;
                lblOpenStatus->SetColor(220, 80, 80);
                ty += lh + 10.f;

                float bw = (mw - pad * 3.f) * 0.5f;
                auto* btnOK = p->Add<Button>(pad, ty, bw, lh + 4.f, "Open");
                btnOK->onClick = [this]{
                    if (!tfOpenPath) return;
                    doOpenProject(tfOpenPath->text);
                };
                auto* btnCancel = p->Add<Button>(pad * 2.f + bw, ty, bw, lh + 4.f, "Cancel");
                btnCancel->onClick = [this]{ pOpenProjModal->visible = false; };
            }
        );
    }

    // Helper — build a centred modal panel and add it as an overlay
    Panel* buildModalPanel(const char* title, std::function<void(Panel*)> populate,
                           float mh = 220.f)
    {
        float mw = 420.f;
        float mx = (kW - mw) * 0.5f, my = (kH - mh) * 0.5f;
        auto* p = ui.AddRoot<Panel>(mx, my, mw, mh, title);
        p->visible = false;
        p->zOrder  = 200;
        ui.BringToFront(p);
        populate(p);
        return p;
    }

    void openNewProjectModal()
    {
        if (pNewProjModal)  { pNewProjModal->visible  = true; if (lblNewStatus)  lblNewStatus->SetText(""); }
    }
    void openOpenProjectModal()
    {
        if (pOpenProjModal) { pOpenProjModal->visible = true; if (lblOpenStatus) lblOpenStatus->SetText(""); }
    }

    // ── Project operations ────────────────────────────────────────────────

    void doCreateProject(const std::string& folderPath, const std::string& name,
                         int tplIdx = 0)
    {
        ProjectTemplate tpl = (tplIdx == 1) ? ProjectTemplate::Game2D :
                              (tplIdx == 2) ? ProjectTemplate::Game3D  :
                                             ProjectTemplate::Empty;
        if (!pm.Create(folderPath, name, tpl)) {
            if (lblNewStatus) lblNewStatus->SetText("Failed to create project.");
            return;
        }
        currentScenePath = "scenes/main.lescene";
        resetTabsToSceneOnly();
        switchToEditor();
    }

    void doOpenProject(const std::string& path)
    {
        if (!pm.Open(path)) {
            if (lblOpenStatus) lblOpenStatus->SetText("Cannot open project.");
            return;
        }
        currentScenePath = pm.project.lastScene.empty()
                         ? "scenes/main.lescene"
                         : pm.project.lastScene;
        // Load scene if it exists
        std::string absScene = pm.AbsScene(currentScenePath);
        if (fs::exists(absScene))
            pm.LoadScene(currentScenePath, editorLevel);

        resetTabsToSceneOnly();

        switchToEditor();
    }

    void switchToEditor()
    {
        state = State::Editor;
        cachePersistenceSuspended = true;
        ui.ClearRoots();
        clearEditorPtrs();
        resetTabsToSceneOnly();
        buildEditorUI();
        rebuildHierarchyTree();
        refreshContentBrowser();
        cachePersistenceSuspended = false;
    }

    void rebuildSplash()
    {
        ui.ClearRoots();
        clearEditorPtrs();
        buildSplash();
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Editor UI
    // ─────────────────────────────────────────────────────────────────────────

    void buildEditorUI()
    {
        buildTitleBar();
        buildMenuBar();
        buildToolbar();
        buildDockSpace();
        buildScriptDock();
        buildFooter();
        buildContextMenus();
        switchLayout(tabManager.HasActiveDocument());
        refreshDocumentWorkspace();
    }

    void rebuildEditorUI()
    {
        cachePersistenceSuspended = true;
        int   selTabIdx = tabManager.ActiveIndex();
        ui.ClearRoots();
        clearEditorPtrs();
        tabManager.Activate(selTabIdx);
        buildEditorUI();
        rebuildHierarchyTree();
        refreshContentBrowser();
        cachePersistenceSuspended = false;
    }

    void clearEditorPtrs()
    {
        pSplash = nullptr; pNewProjModal = nullptr; pOpenProjModal = nullptr;
        tfNewName = nullptr; tfNewPath = nullptr; tfOpenPath = nullptr;
        lblNewStatus = nullptr; lblOpenStatus = nullptr;
        pTabStrip = nullptr; pMenuBar = nullptr; pToolbar = nullptr;
        pDockSpace = nullptr; pBottomTrayNode = nullptr; pCamWidget = nullptr; pContentBrow = nullptr;
        pHierarchy = nullptr; pInspector = nullptr; inspLastW = 0.f;
        pViewport = nullptr; pHierTree = nullptr; pCBTree = nullptr; pCBScroll = nullptr; pCBGrid = nullptr;
        cbTreeNodePaths.clear(); cbLastPanelW = 0.f; cbLastPanelH = 0.f;
        pPluginWizardModal = nullptr;
        tfPluginName = nullptr; tfPluginId = nullptr; tfPluginVersion = nullptr;
        tfPluginCategory = nullptr; tfPluginSubcat = nullptr;
        lblPluginWizardStat = nullptr;
        pScriptPanel = nullptr; pScriptEdit = nullptr;
        pScriptDock = nullptr; pDocumentFilesPanel = nullptr;
        pDocumentOutlinePanel = nullptr; pDocumentEditorPanel = nullptr;
        pScriptDockEdit = nullptr;
        pConsolePanel = nullptr; pConsoleSV = nullptr;
        pCBRenameModal = nullptr; tfCBRename = nullptr; lblCBRenameStat = nullptr;
        pCBPropsModal = nullptr;
        pFooter = nullptr; lblFooterLeft = nullptr; lblFooterRight = nullptr;
        hierMenu = nullptr; cbMenu = nullptr; lblFps = nullptr;
        // Release editor-loaded texture when the UI tree is rebuilt
        if (texEditorTex.IsValid()) { texEditorTex.Release(); texEditorPath.clear(); }
    }

    // ── Title bar ─────────────────────────────────────────────────────────
    void buildTitleBar()
    {
        static constexpr float kLogoPad = 3.f;
        static constexpr float kLogoSz  = kTitleH - kLogoPad * 2.f;
        float tsW = kW - kLogoW - kProjNameW;

        auto* logoImg = ui.AddRoot<Image>(kLogoPad, kLogoPad, kLogoSz, kLogoSz, &logoTex);
        logoImg->zOrder = 50;

        pTabStrip = ui.AddRoot<TabStrip>(kLogoW, 0.f, tsW, kTitleH, kTitleH);
        pTabStrip->zOrder  = 50;
        pTabStrip->tabPadX = 9.f;

        syncTabStripFromManager();

        pTabStrip->onTabChanged = [this](int i) { activateTabIndex(i); };
        pTabStrip->onClose = [this](int i) { closeTab(i); };
    }

    void switchLayout(bool toDocument)
    {
        if (pDockSpace)   pDockSpace->visible   = !toDocument;
        if (pScriptDock)  pScriptDock->visible  =  toDocument;
    }

    void closeTab(int idx)
    {
        tabManager.Close(idx);
        syncTabStripFromManager();
        switchLayout(tabManager.HasActiveDocument());
        refreshDocumentWorkspace();
    }

    // ── Menu bar ──────────────────────────────────────────────────────────
    void buildMenuBar()
    {
        pMenuBar = ui.AddRoot<MenuBar>(0.f, kTitleH, kW, kMenuH);
        pMenuBar->zOrder = 50;

        // FILE
        pMenuBar->AddMenu("FILE");
        pMenuBar->AddItem("FILE", "New Project",  [this]{ closeAndGoSplash(); openNewProjectModal(); });
        pMenuBar->AddItem("FILE", "New Scene",    [this]{ newScene(); });
        pMenuBar->AddItem("FILE", "Open Project", [this]{ closeAndGoSplash(); openOpenProjectModal(); });
        pMenuBar->AddItem("FILE", "Open Scene",   [this]{ openSceneDialog(); });
        pMenuBar->AddSeparator("FILE");
        pMenuBar->AddItem("FILE", "Plugins -> Adicionar",    [this]{ addPluginToProject(); });
        pMenuBar->AddItem("FILE", "Plugins -> Criar Wizard", [this]{ openPluginWizardModal(); });
        pMenuBar->AddSeparator("FILE");
        pMenuBar->AddItem("FILE", "Save Files",   [this]{ saveScene(); saveDirtyDocuments(); });
        pMenuBar->AddItem("FILE", "Save All",     [this]{ saveScene(); saveDirtyDocuments(); pm.Save(); });
        pMenuBar->AddSeparator("FILE");
        pMenuBar->AddItem("FILE", "Close Project",[this]{ closeAndGoSplash(); });
        pMenuBar->AddItem("FILE", "Exit",         [this]{ Quit(); });

        // EDIT
        pMenuBar->AddMenu("EDIT");
        pMenuBar->AddItem("EDIT", "Select All",  []{});
        pMenuBar->AddItem("EDIT", "Undo",        [this]{ undoStack.Undo(); });
        pMenuBar->AddItem("EDIT", "Redo",        [this]{ undoStack.Redo(); });
        pMenuBar->AddItem("EDIT", "Cut",         []{});
        pMenuBar->AddItem("EDIT", "Copy",        []{});
        pMenuBar->AddItem("EDIT", "Paste",       []{});
        pMenuBar->AddSeparator("EDIT");
        pMenuBar->AddItem("EDIT", "Duplicate",   [this]{ duplicateSelected(); });
        pMenuBar->AddItem("EDIT", "Delete",      [this]{ deleteSelected(); });
        pMenuBar->AddSeparator("EDIT");
        pMenuBar->AddItem("EDIT", "Move Forward",  []{});
        pMenuBar->AddItem("EDIT", "Move Backward", []{});
        pMenuBar->AddItem("EDIT", "Move Left",     []{});
        pMenuBar->AddItem("EDIT", "Move Right",    []{});
        pMenuBar->AddItem("EDIT", "Move Up",       []{});
        pMenuBar->AddItem("EDIT", "Move Down",     []{});

        // TOOLS
        pMenuBar->AddMenu("TOOLS");
        pMenuBar->AddItem("TOOLS", "Open Atlas",        []{});
        pMenuBar->AddItem("TOOLS", "Sprite Tool",       []{});
        pMenuBar->AddSeparator("TOOLS");
        pMenuBar->AddItem("TOOLS", "Open Equinox",      [this]{ openEquinoxWorkspace(); });
        pMenuBar->AddItem("TOOLS", "Open Hurricane",    []{});
        pMenuBar->AddItem("TOOLS", "Open Level Script", []{});
        pMenuBar->AddSeparator("TOOLS");
        pMenuBar->AddItem("TOOLS", "Profiler",          []{});

        // WINDOW
        pMenuBar->AddMenu("WINDOW");
        pMenuBar->AddItem("WINDOW", "Project",         []{});
        pMenuBar->AddItem("WINDOW", "Content Browser", []{});
        pMenuBar->AddItem("WINDOW", "Assets",          []{});
        pMenuBar->AddItem("WINDOW", "Hierarchy View",  []{});
        pMenuBar->AddItem("WINDOW", "Properties",      []{});
        pMenuBar->AddItem("WINDOW", "Console",         [this]{ toggleConsole(); });

        // HELP
        pMenuBar->AddMenu("HELP");
        pMenuBar->AddItem("HELP", "Documentation", []{});
        pMenuBar->AddItem("HELP", "About",         []{});
    }

    // ── Toolbar ───────────────────────────────────────────────────────────
    void buildToolbar()
    {
        float ty = kTitleH + kMenuH;
        pToolbar = ui.AddRoot<Toolbar>(0.f, ty, kW, kToolH);
        pToolbar->zOrder = 50;
        pToolbar->AddButton("Salvar", [this](bool){ saveScene(); saveDirtyDocuments(); }, false);
        pToolbar->AddButton("Play/Pause", [this](bool act){
            isPlaying = act;
            // On stop: restore scene to last saved state
            if (!act && pm.isOpen) {
                std::string abs = pm.AbsScene(currentScenePath);
                if (fs::exists(abs)) {
                    pm.LoadScene(currentScenePath, editorLevel);
                    selectedNode = nullptr;
                    rebuildHierarchyTree();
                    refreshInspector();
                }
            }
        }, true);
        pToolbar->AddButton("Compilar", [this](bool){
            if (!pm.isOpen || currentScenePath.empty()) {
                Logger::LogWarning("[Editor] No scene open to compile.");
                return;
            }
            saveScene();
            Logger::LogInfo("[Editor] Scene saved for compile request.");
        }, false);
        pToolbar->AddButton("Config Cena", [](bool){}, false);
        pToolbar->AddButton("Ferramentas", [](bool){}, false);
        pToolbar->AddSeparator();
        pToolbar->AddButton("Grid", [this](bool active){ viewportShowGrid = active; noteEngineChange("Viewport grid toggled"); }, true);
        pToolbar->items.back().active = viewportShowGrid;
        pToolbar->AddButton("Snap", [this](bool active){ viewportSnapToGrid = active; noteEngineChange("Viewport snap toggled"); }, true);
        pToolbar->items.back().active = viewportSnapToGrid;
        pToolbar->AddButton("Enquadrar", [this](bool){ frameViewportSelection(); }, false);
        pToolbar->AddButton("Reset View", [this](bool){ resetViewportView(); }, false);
        syncViewportToolbarState();
    }

    // ── DockSpace ─────────────────────────────────────────────────────────
    void buildDockSpace()
    {
        const float workspaceH = kMainH + kScriptH;
        pDockSpace = ui.AddRoot<DockSpace>(0.f, kTopH, kW, workspaceH);
        DockNode* root = pDockSpace->Root();

        float topFrac = kMainH / workspaceH;
        auto [topNode, bottomNode] = root->Split(false, topFrac);
        pBottomTrayNode = bottomNode;
        pDockSpace->SetTrayNode(bottomNode);

        float leftFrac  = kLeftW / kW;
        auto [left, centerRight] = topNode->Split(true, leftFrac);

        float rightFrac = kHierW / (kW - kLeftW);
        auto [center, right] = centerRight->Split(true, 1.f - rightFrac);

        left->Dock(buildHierarchy(),            "Hierarchy View");
        center->Dock(buildViewportWidget(),     "Viewport");
        right->Dock(buildInspector(),           "Properties");
        bottomNode->Dock(buildContentBrowser(), "Content Browser");
        bottomNode->Dock(buildConsolePanel(),   "Console");
        bottomNode->activeIdx = std::clamp(cacheBottomTrayActiveIdx, 0, (int)bottomNode->panels.size() - 1);
        bottomNode->applyPanelGeometry();

        left->SetLayout(pHierarchy, [this](float w, float h) {
            float pad = gStyle.padding;
            if (pHierTree) { pHierTree->w = w - pad * 2.f; pHierTree->h = h - pHierTree->y - pad; }
            noteEngineChange("Hierarchy layout changed");
        });
        center->SetLayout(pViewport, [this](float w, float h) {
            if (pViewport) { pViewport->w = w; pViewport->h = h; }
            noteEngineChange("Viewport layout changed");
        });
        bottomNode->SetLayout(pContentBrow, [this](float w, float h) {
            cbLastPanelW = w;
            cbLastPanelH = h;

            if (pContentBrow) {
                pContentBrow->w = w;
                pContentBrow->h = h;
            }
            if (pCBScroll && pCBTree) {
                float pad = gStyle.padding;
                float lh  = gStyle.lineH;
                float headerH = (lh + 4.f) + (lh + 6.f) + (lh + 4.f);
                float topY = pad + headerH;
                float bodyH = h - topY - pad;
                float treeW = std::max(170.f, std::min(300.f, w * 0.26f));
                float splitGap = 6.f;
                float rightX = pad + treeW + splitGap;
                float rightW = w - rightX - pad;

                pCBTree->x = pad;
                pCBTree->y = topY;
                pCBTree->w = treeW;
                pCBTree->h = bodyH;

                pCBScroll->x = rightX;
                pCBScroll->y = topY;
                pCBScroll->w = rightW;
                pCBScroll->h = bodyH;

                if (pCBGrid) {
                    pCBGrid->w = rightW - 8.f;
                    pCBGrid->MarkDirty();
                }
            }
            noteEngineChange("Content browser layout changed");
        });
        bottomNode->SetLayout(pConsolePanel, [this](float w, float h) {
            float pad = gStyle.padding;
            if (pConsoleSV) { pConsoleSV->w = w - pad * 2.f; pConsoleSV->h = h - pConsoleSV->y - pad; }
            noteEngineChange("Console layout changed");
        });
        right->SetLayout(pInspector, [this](float w, float /*h*/) {
            if (std::abs(w - inspLastW) > 0.5f) { inspLastW = w; refreshInspector(); noteEngineChange("Inspector layout changed"); }
        });
        if (pInspector && pInspector->w > 0.f) {
            inspLastW = pInspector->w;
            refreshInspector();
        }
    }

    // ── Camera widget ──────────────────────────────────────────────────────
    std::unique_ptr<Widget> buildCameraWidget()
    {
        auto panel = std::make_unique<Panel>(0.f, 0.f, kLeftW, kCamH, "", false);
        pCamWidget = panel.get();

        const float lh  = gStyle.lineH;
        const float rX  = 90.f;
        const float gSz = 15.f;
        const float hw  = (kLeftW - rX - gStyle.padding - 3.f) / 2.f;
        float ty = 4.f;

        pCamWidget->Add<Button>(kLeftW - gSz - 4.f, ty, gSz, gSz, "S")->onClick = []{};
        pCamWidget->Add<Button>(rX, ty, kLeftW - rX - gSz - 8.f, lh, "MAX CINEMATIC")->onClick = []{};
        ty += lh + 3.f;

        lblFps = pCamWidget->Add<Label>(rX, ty + (lh - ui.font.GlyphH()) * 0.5f, "0.0 fps");
        lblFps->SetColor(gStyle.textGreen.r, gStyle.textGreen.g, gStyle.textGreen.b);
        pCamWidget->Add<Button>(rX + hw + 3.f, ty, hw, lh, "RTRT")->onClick = []{};
        ty += lh + 3.f;

        auto* lRes = pCamWidget->Add<Label>(rX, ty + (lh - ui.font.GlyphH()) * 0.5f, "100% RES");
        lRes->SetColor(gStyle.textDim.r, gStyle.textDim.g, gStyle.textDim.b);
        pCamWidget->Add<Label>(rX + hw + 3.f, ty + (lh - ui.font.GlyphH()) * 0.5f, "1280p")
                  ->SetColor(gStyle.textDim.r, gStyle.textDim.g, gStyle.textDim.b);
        ty += lh + 3.f;

        pCamWidget->Add<Label>(rX, ty + (lh - ui.font.GlyphH()) * 0.5f, "ILLUMINADO")
                  ->SetColor(210, 185, 75);
        ty += lh + 3.f;

        pCamWidget->Add<Label>(rX, ty + (lh - ui.font.GlyphH()) * 0.5f, "ORTHO  PERSPEC")
                  ->SetColor(gStyle.textDim.r, gStyle.textDim.g, gStyle.textDim.b);

        return panel;
    }

    // ── Content Browser ───────────────────────────────────────────────────
    // Lists files from pm.AssetsDir(). Refreshed after project open/close.
    std::unique_ptr<Widget> buildContentBrowser()
    {
        auto panel = std::make_unique<Panel>(0.f, 0.f, kW, kScriptH, "");
        panel->clipChildren = true;
        pContentBrow = panel.get();
        refreshContentBrowser();
        return panel;
    }

    void refreshContentBrowser()
    {
        if (!pContentBrow) return;
        pContentBrow->Clear();
        pCBTree = nullptr;
        pCBScroll = nullptr;
        pCBGrid = nullptr;
        cbTreeNodePaths.clear();

        const float pad = gStyle.padding;
        const float lh  = gStyle.lineH;
        const float panelW = (pContentBrow->w > 0.f) ? pContentBrow->w : kW;
        const float panelH = (pContentBrow->h > 0.f) ? pContentBrow->h : kScriptH;
        cbLastPanelW = panelW;
        cbLastPanelH = panelH;
        float ty = pad;

        pContentBrow->Add<Label>(pad, ty, "Content Browser")
            ->SetColor(gStyle.textBright.r, gStyle.textBright.g, gStyle.textBright.b);

        float btnW = 70.f;
        auto* btnUp = pContentBrow->Add<Button>(panelW - pad - btnW, ty - 1.f, btnW, lh + 2.f, "Subir");
        btnUp->onClick = [this] { goContentParent(); };
        ty += lh + 4.f;

        auto* btnRefresh = pContentBrow->Add<Button>(pad, ty, 80.f, lh + 2.f, "Atualizar");
        btnRefresh->onClick = [this] { refreshContentBrowser(); };
        auto* btnImport = pContentBrow->Add<Button>(pad + 84.f, ty, 80.f, lh + 2.f, "Importar");
        btnImport->onClick = [this] { importFileToCurrentDir(); };
        auto* btnFolder = pContentBrow->Add<Button>(pad + 168.f, ty, 96.f, lh + 2.f, "Nova Pasta");
        btnFolder->onClick = [this] { createFolderInCurrentDir(); };

        auto* ddType = pContentBrow->Add<Dropdown>(panelW - pad - btnW - 228.f, ty, 120.f, lh + 2.f, "Tipo");
        ddType->AddItem("All");
        ddType->AddItem("Images");
        ddType->AddItem("Scripts");
        ddType->AddItem("Scenes");
        ddType->AddItem("Shaders");
        ddType->AddItem("Config");
        ddType->SetSelected(cbTypeFilter);
        ddType->onSelect = [this](int idx, const std::string&) {
            cbTypeFilter = idx;
            refreshContentBrowser();
            noteEngineChange("Content browser type filter changed");
        };

        auto* ddView = pContentBrow->Add<Dropdown>(panelW - pad - btnW - 102.f, ty, 96.f, lh + 2.f, "View");
        ddView->AddItem("Grid");
        ddView->AddItem("List");
        ddView->SetSelected(cbViewMode);
        ddView->onSelect = [this](int idx, const std::string&) {
            cbViewMode = idx;
            refreshContentBrowser();
            noteEngineChange("Content browser view mode changed");
        };
        ty += lh + 6.f;

        if (!pm.isOpen) {
            auto* e = pContentBrow->Add<Label>(pad, ty, "(no project)");
            e->h = lh;
            e->SetColor(gStyle.textDim.r, gStyle.textDim.g, gStyle.textDim.b);
            return;
        }

        ensureCBDirValid();
        if (!cbSelectedPath.empty()) {
            std::error_code sec;
            if (!fs::exists(fs::path(cbSelectedPath), sec)) {
                cbSelectedPath.clear();
                cbLastClickPath.clear();
            }
        }
        fs::path root = contentRootDir();
        fs::path cur(cbCurrentDir);
        std::error_code ec;
        std::string crumb = "Root";
        fs::path rel = fs::relative(cur, root, ec);
        if (!ec && !rel.empty() && rel.string() != ".") {
            for (const auto& part : rel) crumb += " > " + part.string();
        }

        auto* pathLbl = pContentBrow->Add<Label>(pad, ty, crumb.c_str());
        pathLbl->h = lh;
        pathLbl->SetColor(gStyle.textDim.r, gStyle.textDim.g, gStyle.textDim.b);
        ty += lh + 4.f;

        float bodyH = panelH - ty - pad;
        float treeW = std::max(170.f, std::min(300.f, panelW * 0.26f));
        float splitGap = 6.f;
        float rightX = pad + treeW + splitGap;
        float rightW = panelW - rightX - pad;

        pCBTree = pContentBrow->Add<TreeView>(pad, ty, treeW, bodyH);
        pCBTree->showRoot = true;
        pCBTree->itemH = lh + 2.f;
        pCBTree->root.label = fs::path(pm.project.rootPath).filename().string();
        if (pCBTree->root.label.empty()) pCBTree->root.label = "Project";
        pCBTree->root.expanded = true;
        cbTreeNodePaths.push_back({ &pCBTree->root, root.string() });
        buildCBTreeRecursive(&pCBTree->root, root);
        pCBTree->onSelect = [this](TreeNode* tn) {
            std::string path = cbTreePathForNode(tn);
            if (!path.empty()) openContentDirectory(fs::path(path));
        };

        pCBScroll = pContentBrow->Add<ScrollView>(rightX, ty, rightW, bodyH);
        pCBScroll->autoContent = true;

        std::vector<fs::directory_entry> dirs;
        std::vector<fs::directory_entry> files;
        try {
            for (auto& entry : fs::directory_iterator(cur)) {
                if (entry.is_directory()) dirs.push_back(entry);
                else if (entry.is_regular_file()) files.push_back(entry);
            }
        } catch (...) {
            Logger::LogWarning("[Editor] Content Browser: failed to list folder");
        }

        auto byName = [](const fs::directory_entry& a, const fs::directory_entry& b) {
            return LightningEditor::ToLowerCopy(a.path().filename().string()) <
                   LightningEditor::ToLowerCopy(b.path().filename().string());
        };
        std::sort(dirs.begin(), dirs.end(), byName);
        std::sort(files.begin(), files.end(), byName);

        if (cbViewMode == 0) {
            float cellW = 110.f;
            float gap = 8.f;
            int cols = std::max(1, (int)((rightW - gap) / (cellW + gap)));
            float finalCellW = (rightW - gap * (cols + 1)) / cols;

            pCBGrid = pCBScroll->Add<Grid>(0.f, 0.f, rightW - 8.f, cols, 84.f, gap, gap, gap, gap);
            pCBGrid->stretchCells = true;
            pCBGrid->autoH = true;

            for (const auto& d : dirs) {
                std::string label = d.path().filename().string();
                auto* btn = pCBGrid->Add<Button>(0.f, 0.f, finalCellW, 84.f, label.c_str());
                btn->icon = resolveCBDirectoryIcon(d.path());
                btn->iconTop = true;
                btn->iconSize = 28.f;
                btn->SetColor(125, 170, 255);
                if (cbSelectedPath == d.path().string()) btn->SetColor(255, 220, 120);
                btn->onClick = [this, d] { handleCBEntryClick(d.path(), true); };
            }

            for (const auto& f : files) {
                if (!matchesCBTypeFilter(f.path())) continue;
                std::string ext = f.path().extension().string();
                std::string lowerExt = LightningEditor::ToLowerCopy(ext);
                std::string name = f.path().filename().string();
                std::string label = name;
                if (label.size() > 16) label = label.substr(0, 13) + "...";
                auto* btn = pCBGrid->Add<Button>(0.f, 0.f, finalCellW, 84.f, label.c_str());
                btn->icon = resolveCBFileIcon(f.path());
                btn->iconTop = true;
                btn->iconSize = 28.f;
                if (lowerExt == ".lescene") {
                    btn->SetColor(75, 195, 75);
                } else {
                    auto tab = LightningEditor::BuildAssetTab(f.path().string());
                    btn->SetColor(tab.accent.r, tab.accent.g, tab.accent.b);
                }
                if (cbSelectedPath == f.path().string()) btn->SetColor(255, 220, 120);
                btn->onClick = [this, f, ext] {
                    handleCBEntryClick(f.path(), false);
                    cbDragFile = f.path().string();
                    cbDragExt  = ext;
                    cbDragging = true;
                };
            }
        } else {
            auto addDirRow = [this, rightW](const fs::path& absDir) {
                float rowH = gStyle.lineH + 2.f;
                auto* btn = pCBScroll->Add<Button>(0.f, 0.f, rightW, rowH,
                                                   absDir.filename().string().c_str());
                btn->icon = resolveCBDirectoryIcon(absDir);
                btn->iconTop = false;
                btn->iconSize = 14.f;
                btn->SetColor(125, 170, 255);
                if (cbSelectedPath == absDir.string()) btn->SetColor(255, 220, 120);
                btn->onClick = [this, absDir] { handleCBEntryClick(absDir, true); };
            };
            auto addFileRow = [this, rightW](const fs::path& absFile) {
                float rowH = gStyle.lineH + 2.f;
                std::string ext = absFile.extension().string();
                auto* btn = pCBScroll->Add<Button>(0.f, 0.f, rightW, rowH,
                                                   absFile.filename().string().c_str());
                btn->icon = resolveCBFileIcon(absFile);
                btn->iconTop = false;
                btn->iconSize = 14.f;
                auto tab = LightningEditor::BuildAssetTab(absFile.string());
                btn->SetColor(tab.accent.r, tab.accent.g, tab.accent.b);
                if (cbSelectedPath == absFile.string()) btn->SetColor(255, 220, 120);
                btn->onClick = [this, absFile, ext] {
                    handleCBEntryClick(absFile, false);
                    cbDragFile = absFile.string();
                    cbDragExt  = ext;
                    cbDragging = true;
                };
            };

            for (const auto& d : dirs) addDirRow(d.path());
            for (const auto& f : files) {
                if (!matchesCBTypeFilter(f.path())) continue;
                addFileRow(f.path());
            }
        }

    }

    // ── Hierarchy ─────────────────────────────────────────────────────────
    std::unique_ptr<Widget> buildHierarchy()
    {
        auto panel = std::make_unique<Panel>(0.f, 0.f, kHierW, kHierSplitH, "");
        pHierarchy = panel.get();

        float pad = gStyle.padding, lh = gStyle.lineH, ty = pad;

        pHierarchy->Add<Label>(pad, ty + (lh - ui.font.GlyphH()) * 0.5f, "Scene")
                  ->SetColor(gStyle.textBright.r, gStyle.textBright.g, gStyle.textBright.b);
        ty += lh + 2.f;

        auto* btnAdd = pHierarchy->Add<Button>(kHierW - pad - 60.f, pad, 60.f, lh, "+ Node");
        btnAdd->onClick = [this]{ addNodeToScene("Node"); };

        pHierarchy->Add<Button>(pad, ty, kHierW - pad * 2.f, lh + 2.f, "Search...")->onClick = []{};
        ty += lh + 6.f;

        float svH = kHierSplitH - ty - pad;
        pHierTree = pHierarchy->Add<TreeView>(pad, ty, kHierW - pad * 2.f, svH);
        pHierTree->showRoot = false;
        pHierTree->onSelect = [this](TreeNode* tn) {
            if (!tn) { selectedNode = nullptr; refreshInspector(); return; }
            selectedNode = reinterpret_cast<Node*>(tn->userData);
            refreshInspector();
        };

        return panel;
    }

    void rebuildHierarchyTree()
    {
        if (!pHierTree) return;
        pHierTree->root.ClearChildren();
        pHierTree->selected = nullptr;

        for (auto& n : editorLevel.GetNodes())
            addNodeToTree(&pHierTree->root, n.get());
    }

    void addNodeToTree(TreeNode* parent, Node* node)
    {
        if (!node) return;
        // Tag line shows component count (rough hint)
        std::string tag = node->tag.empty() ? node->name : node->tag;
        TreeNode* tn    = parent->AddChild(node->name, tag);
        tn->userData    = reinterpret_cast<void*>(node);
        tn->expanded    = true;
        if (node == selectedNode) pHierTree->selected = tn;
        for (Node* child : node->GetChildren())
            addNodeToTree(tn, child);
    }

    // ── Inspector ─────────────────────────────────────────────────────────
    std::unique_ptr<Widget> buildInspector()
    {
        auto panel = std::make_unique<Panel>(0.f, 0.f, kHierW, kInspH, "");
        pInspector = panel.get();
        refreshInspector();
        return panel;
    }

    void refreshInspector()
    {
        if (!pInspector) return;
        pInspector->Clear();

        const float pad    = gStyle.padding;
        const float lh     = gStyle.lineH;
        const float panelW = (pInspector->w > 0.f) ? pInspector->w : kHierW;
        const float innerW = panelW - pad * 2.f;
        auto* stack = pInspector->Add<VStack>(pad, pad, innerW, 3.f);

        if (!selectedNode) {
            auto* lbl = stack->Add<Label>(0.f, 0.f, "No selection");
            lbl->h = lh;
            lbl->SetColor(gStyle.textDim.r, gStyle.textDim.g, gStyle.textDim.b);
            return;
        }

        Node* node = selectedNode;

        // Name
        stack->Add<Label>(0.f, 0.f, "Name")->h = lh;
        auto* tfName = stack->Add<TextField>(0.f, 0.f, innerW, lh + 2.f);
        tfName->SetText(node->name);
        tfName->onChanged = [this, node](const std::string& v) {
            node->name = v;
            rebuildHierarchyTree();
        };
        stack->Add<Separator>(innerW);

        // Tag
        stack->Add<Label>(0.f, 0.f, "Tag")->h = lh;
        auto* tfTag = stack->Add<TextField>(0.f, 0.f, innerW, lh + 2.f);
        tfTag->SetText(node->tag);
        tfTag->onChanged = [node](const std::string& v) { node->tag = v; };
        stack->Add<Separator>(innerW);

        // Active
        auto* chk = stack->Add<Checkbox>(0.f, 0.f, "Active", node->active);
        chk->h = lh;
        chk->onChange = [node](bool v) { node->active = v; };
        stack->Add<Separator>(innerW);

        // Transform — Position
        {
            auto* lbl = stack->Add<Label>(0.f, 0.f, "Position");
            lbl->h = lh;
            lbl->SetColor(gStyle.textBright.r, gStyle.textBright.g, gStyle.textBright.b);

            struct Axis { const char* name; Uint8 r,g,b; float* ptr; };
            Axis axes[] = {
                { "X", 220, 80,  80,  &node->transform.Position.x },
                { "Y", 80,  200, 80,  &node->transform.Position.y },
                { "Z", 80,  120, 220, &node->transform.Position.z },
            };
            for (auto& a : axes) {
                auto* row = stack->Add<HStack>(0.f, 0.f, innerW, lh, 3.f);
                row->stretchH = true;
                auto* al = row->Add<Label>(0.f, 0.f, a.name);
                al->w = 14.f; al->h = lh;
                al->SetColor(a.r, a.g, a.b);
                float* ptr = a.ptr;
                auto* num = row->Add<NumericUpDown>(0.f, 0.f, innerW - 14.f, lh,
                                                    *ptr, -99999.f, 99999.f, 1.f, 1);
                num->onChanged = [this, ptr](float v) {
                    float old = *ptr;
                    undoStack.Do([ptr, v]{ *ptr = v; }, [ptr, old]{ *ptr = old; }, "Move");
                };
            }
        }
        stack->Add<Separator>(innerW);

        // Rotation
        {
            auto* lbl = stack->Add<Label>(0.f, 0.f, "Rotation");
            lbl->h = lh;
            lbl->SetColor(gStyle.textBright.r, gStyle.textBright.g, gStyle.textBright.b);

            struct Axis { const char* name; Uint8 r,g,b; float* ptr; };
            Axis axes[] = {
                { "X", 220, 80,  80,  &node->transform.Rotation.x },
                { "Y", 80,  200, 80,  &node->transform.Rotation.y },
                { "Z", 80,  120, 220, &node->transform.Rotation.z },
            };
            for (auto& a : axes) {
                auto* row = stack->Add<HStack>(0.f, 0.f, innerW, lh, 3.f);
                row->stretchH = true;
                auto* al = row->Add<Label>(0.f, 0.f, a.name);
                al->w = 14.f; al->h = lh;
                al->SetColor(a.r, a.g, a.b);
                float* ptr = a.ptr;
                auto* num = row->Add<NumericUpDown>(0.f, 0.f, innerW - 14.f, lh,
                                                    *ptr, -360.f, 360.f, 1.f, 1);
                num->onChanged = [this, ptr](float v) {
                    float old = *ptr;
                    undoStack.Do([ptr, v]{ *ptr = v; }, [ptr, old]{ *ptr = old; }, "Rotate");
                };
            }
        }
        stack->Add<Separator>(innerW);

        // Scale
        {
            auto* lbl = stack->Add<Label>(0.f, 0.f, "Scale");
            lbl->h = lh;
            lbl->SetColor(gStyle.textBright.r, gStyle.textBright.g, gStyle.textBright.b);

            struct Axis { const char* name; Uint8 r,g,b; float* ptr; };
            Axis axes[] = {
                { "X", 220, 80,  80,  &node->transform.Scale.x },
                { "Y", 80,  200, 80,  &node->transform.Scale.y },
                { "Z", 80,  120, 220, &node->transform.Scale.z },
            };
            for (auto& a : axes) {
                auto* row = stack->Add<HStack>(0.f, 0.f, innerW, lh, 3.f);
                row->stretchH = true;
                auto* al = row->Add<Label>(0.f, 0.f, a.name);
                al->w = 14.f; al->h = lh;
                al->SetColor(a.r, a.g, a.b);
                float* ptr = a.ptr;
                auto* num = row->Add<NumericUpDown>(0.f, 0.f, innerW - 14.f, lh,
                                                    *ptr, 0.001f, 9999.f, 0.1f, 2);
                num->onChanged = [this, ptr](float v) {
                    float old = *ptr;
                    undoStack.Do([ptr, v]{ *ptr = v; }, [ptr, old]{ *ptr = old; }, "Scale");
                };
            }
        }
        stack->Add<Separator>(innerW);

        // Script (NucleoScript) — show path if component exists
        if (auto* sc = node->GetComponent<NucleoScriptComponent>()) {
            auto* lbl = stack->Add<Label>(0.f, 0.f, "Script");
            lbl->h = lh;
            lbl->SetColor(255, 138, 28);

            auto* tfScript = stack->Add<TextField>(0.f, 0.f, innerW, lh + 2.f);
            tfScript->SetText(sc->scriptPath);
            tfScript->onChanged = [sc](const std::string& v) {
                sc->scriptPath = v;
                sc->Reload();
            };
            if (!sc->lastError.empty()) {
                auto* err = stack->Add<Label>(0.f, 0.f, sc->lastError.c_str());
                err->h = lh;
                err->SetColor(220, 80, 80);
            }
        }

        // Add Script button
        {
            auto* btn = stack->Add<Button>(0.f, 0.f, innerW, lh + 2.f, "+ Add Script Component");
            btn->onClick = [this, node]{
                if (!node->HasComponent<NucleoScriptComponent>())
                    node->AddComponent<NucleoScriptComponent>();
                refreshInspector();
            };
        }
    }

    // ── Viewport — renders the real Level ─────────────────────────────────
    std::unique_ptr<Widget> buildViewportWidget()
    {
        auto vp = std::make_unique<Viewport2D>(0.f, 0.f, kVpW, kVpH);
        pViewport = vp.get();
        pViewport->showBg     = true;
        pViewport->showBorder = true;

        pViewport->onRender = [this](Renderer& r, float ax, float ay, float vw, float vh) {
            // Store viewport screen origin for gizmo hit-testing
            vpAX = ax; vpAY = ay;
            r.BeginScreenSpace();

            r.SetDrawColor(20, 20, 26);
            r.FillRect(ax, ay, vw, vh);

            // Render real scene nodes (positions are in viewport-local screen coords)
            r.EndScreenSpace();
            r.SetScissor(ax, ay, vw, vh);
            r.SetCameraOrigin(ax, ay);
            r.SetCameraOffset(viewportCamX, viewportCamY);
            r.SetCameraZoom(viewportZoom);
            drawViewportGrid(r, vw, vh);
            editorLevel.Render();    // node components draw via renderer (screen-space coords)
            r.ClearScissor();
            r.SetCameraOrigin(0.f, 0.f);
            r.SetCameraOffset(0.f, 0.f);
            r.SetCameraZoom(1.f);
            r.BeginScreenSpace();

            // Gizmos (move handles) — only in editor mode
            if (selectedNode && !isPlaying) {
                static constexpr float kArrow = 50.f;
                static constexpr float kTip   = 5.f;
                auto wp = selectedNode->WorldPosition();
                Lightning::V2 screenPos = viewportWorldToScreen(wp.x, wp.y);
                float nx = screenPos.x;
                float ny = screenPos.y;

                // Center square (white)
                r.SetDrawColor(220, 220, 220, 230);
                r.FillRect(nx - 5.f, ny - 5.f, 10.f, 10.f);

                // X axis — red arrow
                r.SetDrawColor(220, 60, 60, 230);
                r.DrawLine(nx, ny, nx + kArrow, ny, 2.f);
                r.FillCircle(nx + kArrow, ny, kTip);

                // Y axis — green arrow (down = +Y in screen space)
                r.SetDrawColor(60, 200, 60, 230);
                r.DrawLine(nx, ny, nx, ny + kArrow, 2.f);
                r.FillCircle(nx, ny + kArrow, kTip);

                // Selection outline
                r.SetDrawColor(255, 200, 0, 120);
                r.DrawRect(nx - 14.f, ny - 14.f, 28.f, 28.f);
            }

            // Viewport label when scene is empty
            if (editorLevel.NodeCount() == 0) {
                const char* lbl = pm.isOpen ? "Empty Scene — use + Node to add objects"
                                            : "No Project Open";
                float lw = ui.font.MeasureW(lbl);
                r.SetDrawColor(60, 60, 72);
                ui.font.DrawText(r, lbl, ax + (vw - lw) * 0.5f,
                                       ay + vh * 0.5f - ui.font.GlyphH() * 0.5f);
            }

            // Playing indicator
            if (isPlaying) {
                const char* msg = "● PLAYING";
                float mw = ui.font.MeasureW(msg);
                r.SetDrawColor(80, 200, 80, 200);
                ui.font.DrawText(r, msg, ax + vw - mw - 8.f, ay + 6.f);
            }

            char viewportHud[128];
            SDL_snprintf(viewportHud, sizeof(viewportHud),
                         "Zoom %.0f%%  |  Grid %s  |  Snap %s",
                         viewportZoom * 100.f,
                         viewportShowGrid ? "ON" : "OFF",
                         viewportSnapToGrid ? "ON" : "OFF");
            float hudW = ui.font.MeasureW(viewportHud);
            r.SetDrawColor(18, 18, 24, 185);
            r.FillRect(ax + 10.f, ay + 8.f, hudW + 12.f, ui.font.GlyphH() + 8.f);
            r.SetDrawColor(210, 210, 220, 230);
            ui.font.DrawText(r, viewportHud, ax + 16.f, ay + 12.f);

            std::string sceneWatermark = fs::path(currentScenePath).stem().string();
            if (sceneWatermark.empty()) sceneWatermark = "Scene2D";
            float wmW = ui.font.MeasureW(sceneWatermark.c_str());
            r.SetDrawColor(255, 255, 255, 110);
            ui.font.DrawText(r, sceneWatermark.c_str(), ax + vw - wmW - 18.f,
                             ay + vh - ui.font.GlyphH() - 18.f);

            r.EndScreenSpace();
        };

        return vp;
    }

    // ── Context menus ─────────────────────────────────────────────────────
    void registerHierarchyElements()
    {
        auto& reg = EditorElementRegistry::Instance();
        reg.Clear();

        reg.Register({
            "core.hierarchy.add_child_node",
            "Add Child Node",
            "Hierarchy",
            "Nodes",
            EditorElementKind::NodeFactory,
            EditorElementSource::Core,
            10,
            true,
            [this]{ addChildToSelected(); }
        });

        reg.Register({
            "core.hierarchy.add_script_component",
            "Add Script Component",
            "Hierarchy",
            "Components",
            EditorElementKind::ComponentFactory,
            EditorElementSource::Core,
            20,
            true,
            [this]{
                if (selectedNode && !selectedNode->HasComponent<NucleoScriptComponent>()) {
                    selectedNode->AddComponent<NucleoScriptComponent>();
                    refreshInspector();
                }
            }
        });
    }

    void buildHierarchyContextMenuFromRegistry()
    {
        if (!hierMenu) return;

        auto& reg = EditorElementRegistry::Instance();
        auto groupedNode = reg.GroupByCategory(EditorElementKind::NodeFactory);
        auto groupedComp = reg.GroupByCategory(EditorElementKind::ComponentFactory);

        auto appendGrouped = [this](const EditorElementRegistry::CategoryMap& grouped) {
            for (const auto& [category, subMap] : grouped) {
                for (const auto& [subcategory, bucket] : subMap) {
                    for (const auto& d : bucket) {
                        std::string label = category + "/" + subcategory + "/" + d.label;
                        hierMenu->AddItem(label.c_str(), d.action, d.enabled);
                    }
                }
            }
        };

        appendGrouped(groupedNode);
        appendGrouped(groupedComp);
    }

    void revealSelectedOnExplorer()
    {
        if (cbSelectedPath.empty()) return;

        std::error_code ec;
        fs::path selected(cbSelectedPath);
        if (!fs::exists(selected, ec)) {
            Logger::LogWarning("[Editor] Reveal: selected path does not exist.");
            return;
        }

        fs::path openDir = fs::is_directory(selected, ec) ? selected : selected.parent_path();
        if (openDir.empty()) openDir = selected;

        std::string url = "file:///" + openDir.generic_string();
        if (!SDL_OpenURL(url.c_str())) {
            Logger::LogWarning("[Editor] Reveal on Explorer failed.");
        }
    }

    void createInstanceFromSelectedScript()
    {
        if (cbSelectedPath.empty()) return;

        fs::path selected(cbSelectedPath);
        std::error_code ec;
        if (!fs::exists(selected, ec) || !fs::is_regular_file(selected, ec)) return;

        std::string ext = LightningEditor::ToLowerCopy(selected.extension().string());
        if (ext != ".spark" && ext != ".cs") {
            Logger::LogInfo("[Editor] Create Instance: select Ignite/C# script.");
            return;
        }

        const std::string body =
            "PREFAB \"" + selected.stem().string() + "Instance\"\n"
            "NODE Node \"" + selected.stem().string() + "\"\n"
            "  TRANSFORM 0 0 0  0 0 0  1 1 1\n"
            "  TAG \"ScriptInstance\"\n"
            "  ACTIVE 1\n"
            "  SCRIPT \"" + selected.filename().string() + "\"\n"
            "END\n";

        createAssetInCurrentDir(selected.stem().string() + "_Instance", ".lprefab", body);
    }

    void rebuildContentBrowserContextMenu()
    {
        if (!cbMenu) return;

        cbMenu->items.clear();
        cbMenu->hoverIndex = -1;

        cbMenu->AddItem("Importar arquivo aqui", [this]{ importFileToCurrentDir(); });
        cbMenu->AddItem("Nova pasta", [this]{ createFolderInCurrentDir(); });
        cbMenu->AddItem("Renomear", [this]{ beginCBRenameSelected(); }, !cbSelectedPath.empty());
        cbMenu->AddSeparator();
        cbMenu->AddItem("Propriedades", [this]{ openSelectedCBProperties(); }, !cbSelectedPath.empty());
        cbMenu->AddSeparator();

        auto addCreate = [this](const std::string& label,
                                const std::string& stem,
                                const std::string& ext,
                                const std::string& initial) {
            cbMenu->AddItem(label.c_str(), [this, stem, ext, initial]{
                createAssetInCurrentDir(stem, ext, initial);
            });
        };

        addCreate("Add Prefab", "NewPrefab", ".lprefab",
                  "PREFAB \"NewPrefab\"\n"
                  "NODE Node \"Root\"\n"
                  "  TRANSFORM 0 0 0  0 0 0  1 1 1\n"
                  "  TAG \"\"\n"
                  "  ACTIVE 1\n"
                  "END\n");

        addCreate("Scripts/Ignite (Graph)", "NewIgniteScript", ".spark",
                  "class NewIgniteScript {\n"
                  "    void OnStart() {}\n"
                  "    void Update(float dt) {}\n"
                  "}\n");
        addCreate("Scripts/C#", "NewScript", ".cs",
                  "using System;\n\n"
                  "public class NewScript {\n"
                  "    public void OnStart() {}\n"
                  "    public void OnUpdate(float dt) {}\n"
                  "}\n");
        addCreate("Scripts/Enum", "NewEnum", ".cs",
                  "public enum NewEnum {\n"
                  "    None = 0\n"
                  "}\n");
        addCreate("Scripts/Struct", "NewStruct", ".cs",
                  "public struct NewStruct {\n"
                  "    public int Id;\n"
                  "}\n");
        addCreate("Scripts/Interface", "INewInterface", ".cs",
                  "public interface INewInterface {\n"
                  "    void Execute();\n"
                  "}\n");
        addCreate("Scripts/Misselaneus/DataSet", "NewDataSet", ".json", "{\n  \"items\": []\n}\n");
        addCreate("Scripts/Curves/Float Curve", "FloatCurve", ".curve", "type: float\nkeys: []\n");
        addCreate("Scripts/Curves/Linear Curves", "LinearCurve", ".curve", "type: linear\nkeys: []\n");
        addCreate("Scripts/Curves/Color Curves", "ColorCurve", ".curve", "type: color\nkeys: []\n");

        addCreate("Animation/ScriptAnimation", "NewScriptAnimation", ".spark", "class NewScriptAnimation {}\n");
        addCreate("Animation/RIG/ControlRig", "NewControlRig", ".lrig", "rig: control\n");
        addCreate("Animation/RIG/RetargetAnimation", "NewRetarget", ".lretarget", "retarget: {}\n");
        addCreate("Animation/2D/Rig2D", "NewRig2D", ".lrig2d", "rig2d: {}\n");

        addCreate("VisionDirect/Filmaker", "NewFilmaker", ".lfilm", "filmaker: {}\n");
        addCreate("VisionDirect/Cutscene", "NewCutscene", ".lcutscene", "timeline: []\n");
        addCreate("VisionDirect/Capture Scene", "NewCaptureScene", ".lcapture", "capture: {}\n");

        addCreate("AI/Behavior Tree", "NewBehaviorTree", ".btree", "root: Selector\nchildren: []\n");
        addCreate("AI/Blackboard", "NewBlackboard", ".blackboard", "entries: []\n");
        addCreate("AI/Pathfinder", "NewPathfinder", ".json", "{\n  \"grid\": []\n}\n");
        addCreate("AI/Task Rules", "NewTaskRules", ".json", "{\n  \"rules\": []\n}\n");

        addCreate("Textures/2D/Sprites2D", "NewSprite2D", ".sprite", "sprite: {}\n");
        addCreate("Textures/2D/Sprites3D", "NewSprite3D", ".sprite3d", "sprite3d: {}\n");
        addCreate("Textures/2D/AnimationSprites", "NewAnimSprite", ".animsprite", "frames: []\n");
        addCreate("Textures/2D/Billboard", "NewBillboard", ".billboard", "billboard: {}\n");
        addCreate("Textures/2D/TileSet", "NewTileSet", ".tileset", "tiles: []\n");
        addCreate("Textures/2D/TileMap", "NewTileMap", ".tilemap", "layers: []\n");
        addCreate("Textures/Cubemap", "NewCubemap", ".cubemap", "faces: []\n");
        addCreate("Textures/RenderTargets (RT)", "NewRenderTarget", ".rt", "size: 1024x1024\n");
        addCreate("Textures/VolumeTextures", "NewVolumeTexture", ".vtex", "depth: 16\n");
        addCreate("Textures/VirtualTextures", "NewVirtualTexture", ".virttex", "virtual: true\n");

        addCreate("Materials/Material", "NewMaterial", ".lmat", "shader: default\n");
        addCreate("Materials/Function Material", "NewMaterialFunction", ".lmatfunc", "nodes: []\n");
        addCreate("Materials/Instances Material", "NewMaterialInstance", ".lmatinst", "parent: NewMaterial\n");
        addCreate("Materials/Layer Material", "NewMaterialLayer", ".lmatlayer", "layers: []\n");

        cbMenu->AddItem("Equinox/Equinox Shader Composer", [this] {
            createEquinoxAssetInCurrentDir(EquinoxAssetType::ShaderComposer);
        });
        cbMenu->AddItem("Equinox/Texture Generator", [this] {
            createEquinoxAssetInCurrentDir(EquinoxAssetType::TextureGenerator);
        });

        addCreate("Hurricane/Particle System", "NewParticleSystem", ".hurricane", "emitters: []\n");
        addCreate("Hurricane/Fluids Sim", "NewFluidSim", ".fluid", "simulation: fluid\n");

        addCreate("Audio/Spatial Sound", "NewSpatialSound", ".laudio", "spatial: true\n");
        addCreate("Audio/Audio Mixer", "NewAudioMixer", ".lmixer", "buses: []\n");

        addCreate("Level/Scene2D", "NewScene2D", ".lescene", "# Lightning Engine Scene 1.0\n");
        addCreate("Level/Scene3D", "NewScene3D", ".lescene", "# Lightning Engine Scene 1.0\n");

        if (cbSelectedPath.empty()) return;

        fs::path selected(cbSelectedPath);
        std::error_code ec;
        if (!fs::exists(selected, ec) || !fs::is_regular_file(selected, ec)) return;

        std::string ext = LightningEditor::ToLowerCopy(selected.extension().string());
        cbMenu->AddSeparator();
        cbMenu->AddItem("Reveal on Explorer Files", [this]{ revealSelectedOnExplorer(); });

        if (EquinoxFileManager::IsEquinoxAssetPath(selected.string())) {
            cbMenu->AddItem("Equinox/Abrir no Editor", [this, selected] {
                openAssetContextTab(selected.string());
            });
            cbMenu->AddItem("Equinox/Abrir Workspace", [this] {
                openEquinoxWorkspace();
            });
            cbMenu->AddItem("Equinox/Novo Shader Composer", [this] {
                createEquinoxAssetInCurrentDir(EquinoxAssetType::ShaderComposer);
            });
            cbMenu->AddItem("Equinox/Novo Texture Generator", [this] {
                createEquinoxAssetInCurrentDir(EquinoxAssetType::TextureGenerator);
            });
        }

        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga") {
            cbMenu->AddItem("Sprite Actions/Convert in Sprites", [this, selected] {
                createAssetInCurrentDir(selected.stem().string() + "_Sprite", ".sprite",
                                        "source: \"" + selected.filename().string() + "\"\n");
            });
            cbMenu->AddItem("Sprite Actions/Convert in TileSet", [this, selected] {
                createAssetInCurrentDir(selected.stem().string() + "_TileSet", ".tileset",
                                        "source: \"" + selected.filename().string() + "\"\n"
                                        "tiles: []\n");
            });
        }

        if (ext == ".spark" || ext == ".cs") {
            cbMenu->AddItem("Criar Instancia", [this]{ createInstanceFromSelectedScript(); });
            cbMenu->AddItem("Mudar Heranca", [this, selected]{
                Logger::LogInfo("[Editor] Heranca: abra e edite o tipo base em " + selected.filename().string());
                openAssetContextTab(selected.string());
            });
        }
    }

    void buildContextMenus()
    {
        hierMenu = ui.AddRoot<ContextMenu>();
        registerHierarchyElements();
        buildHierarchyContextMenuFromRegistry();
        hierMenu->AddSeparator();
        hierMenu->AddItem("Duplicate",       [this]{ duplicateSelected(); });
        hierMenu->AddSeparator();
        hierMenu->AddItem("Delete",          [this]{ deleteSelected(); });
        ui.BringToFront(hierMenu);

        cbMenu = ui.AddRoot<ContextMenu>();
        rebuildContentBrowserContextMenu();
        ui.BringToFront(cbMenu);
    }

    // ── Content Browser drag & drop ───────────────────────────────────────
    // Tracks a pending drag from the Content Browser to the Viewport.
    // Drop behaviour:
    //   .spark  → add/replace NucleoScriptComponent on selected node
    //   .lescene→ open the scene
    //   .png/.jpg/.bmp → assign to SpriteRenderer (if present), otherwise log
    void processCBDrop()
    {
        if (!cbDragging) return;

        float mx = inputManager.GetMouseX();
        float my = inputManager.GetMouseY();

        // Cancel drag on right-click
        if (inputManager.IsMousePressed(3)) {
            cbDragging = false;
            cbDragFile.clear();
            return;
        }

        // Detect drop: mouse released over the viewport area
        bool mouseUp = !inputManager.IsMouseDown(1);
        if (!mouseUp) return;  // still dragging

        bool overViewport = pViewport &&
            mx >= vpAX && mx < vpAX + pViewport->w &&
            my >= vpAY && my < vpAY + pViewport->h;

        if (overViewport) {
            applyDroppedAsset(cbDragFile, cbDragExt, mx, my);
        }

        cbDragging = false;
        cbDragFile.clear();
    }

    void applyDroppedAsset(const std::string& path, const std::string& ext,
                           float dropX, float dropY)
    {
        if (ext == ".spark") {
            // Assign script to selected node (or create a new node at drop position)
            Node* target = selectedNode;
            if (!target) {
                std::string stem = fs::path(path).stem().string();
                auto node = std::make_unique<Node>(stem);
                Lightning::V2 worldDrop = snapViewportPoint(viewportScreenToWorld(dropX, dropY));
                node->transform.Position = Lightning::V3(worldDrop.x, worldDrop.y, 0.f);
                editorLevel.AddNode(std::move(node));
                target = editorLevel.GetNodes().back().get();
                rebuildHierarchyTree();
            }
            if (!target->HasComponent<NucleoScriptComponent>())
                target->AddComponent<NucleoScriptComponent>();
            auto* sc = target->GetComponent<NucleoScriptComponent>();
            if (sc) sc->SetScript(path);
            refreshInspector();
            Logger::LogInfo("[Editor] Script assigned: " + fs::path(path).filename().string());

        } else if (ext == ".lescene") {
            // Load the scene
            std::string rel = fs::relative(fs::path(path), fs::path(pm.project.rootPath)).string();
            pm.LoadScene(rel, editorLevel);
            currentScenePath = rel;
            syncPrimarySceneTabLabel();
            selectedNode = nullptr;
            rebuildHierarchyTree();
            refreshInspector();
            Logger::LogInfo("[Editor] Scene loaded: " + rel);

        } else if (ext == ".png" || ext == ".jpg" || ext == ".bmp") {
            Logger::LogInfo("[Editor] Texture drop: " + fs::path(path).filename().string() +
                            " (assign via SpriteRenderer)");
        } else {
            Logger::LogInfo("[Editor] Dropped: " + fs::path(path).filename().string());
        }
    }

    // ── Gizmo drag ─────────────────────────────────────────────────────────
    // Called every editor frame. Detects mouse-down on gizmo handles,
    // drags the selected node, and commits an undo record on release.
    void processGizmoDrag()
    {
        if (!selectedNode) return;

        float mx = inputManager.GetMouseX();
        float my = inputManager.GetMouseY();

        static constexpr float kArrow    = 50.f;  // must match onRender constant
        static constexpr float kHitR     = 9.f;   // hit radius around handle tip
        static constexpr float kCenterR  = 7.f;

        auto wp  = selectedNode->WorldPosition();
        Lightning::V2 screenPos = viewportWorldToScreen(wp.x, wp.y);
        float nx = screenPos.x;
        float ny = screenPos.y;

        auto dist = [](float ax, float ay, float bx, float by){
            float dx = ax-bx, dy = ay-by;
            return std::sqrt(dx*dx + dy*dy);
        };

        // ── Mouse pressed: begin drag ─────────────────────────────────────
        if (inputManager.IsMousePressed(1) && gizmoAxis == GizmoAxis::None) {
            bool nearX  = dist(mx, my, nx + kArrow, ny) < kHitR;
            bool nearY  = dist(mx, my, nx, ny + kArrow) < kHitR;
            bool nearXY = dist(mx, my, nx, ny) < kCenterR;

            if (nearX || nearY || nearXY) {
                gizmoAxis    = nearXY ? GizmoAxis::XY : nearX ? GizmoAxis::X : GizmoAxis::Y;
                gizmoDragMX0 = mx;
                gizmoDragMY0 = my;
                gizmoDragNX0 = selectedNode->transform.Position.x;
                gizmoDragNY0 = selectedNode->transform.Position.y;
            }
        }

        // ── Mouse held: apply drag ────────────────────────────────────────
        if (gizmoAxis != GizmoAxis::None && inputManager.IsMouseDown(1)) {
            float zoom = std::max(viewportZoom, 0.001f);
            float dx = (mx - gizmoDragMX0) / zoom;
            float dy = (my - gizmoDragMY0) / zoom;
            if (gizmoAxis == GizmoAxis::X || gizmoAxis == GizmoAxis::XY)
                selectedNode->transform.Position.x = snapViewportValue(gizmoDragNX0 + dx);
            if (gizmoAxis == GizmoAxis::Y || gizmoAxis == GizmoAxis::XY)
                selectedNode->transform.Position.y = snapViewportValue(gizmoDragNY0 + dy);
            refreshInspector();
        }

        // ── Mouse released: commit to undo ────────────────────────────────
        if (gizmoAxis != GizmoAxis::None && !inputManager.IsMouseDown(1)) {
            GizmoAxis  axis    = gizmoAxis;
            float      nx0     = gizmoDragNX0;
            float      ny0     = gizmoDragNY0;
            float      nxFinal = selectedNode->transform.Position.x;
            float      nyFinal = selectedNode->transform.Position.y;
            Node*      node    = selectedNode;

            undoStack.Push(
                [node, axis, nxFinal, nyFinal](){
                    if (axis == GizmoAxis::X || axis == GizmoAxis::XY) node->transform.Position.x = nxFinal;
                    if (axis == GizmoAxis::Y || axis == GizmoAxis::XY) node->transform.Position.y = nyFinal;
                },
                [this, node, axis, nx0, ny0](){
                    if (axis == GizmoAxis::X || axis == GizmoAxis::XY) node->transform.Position.x = nx0;
                    if (axis == GizmoAxis::Y || axis == GizmoAxis::XY) node->transform.Position.y = ny0;
                    refreshInspector();
                },
                "Move Gizmo"
            );

            gizmoAxis = GizmoAxis::None;
            refreshInspector();
        }
    }

    // ── Script panel (bottom-left strip) ──────────────────────────────────
    void buildScriptPanel()
    {
        float sy  = kTopH + kMainH;
        float spW = kW - kConsoleW;
        pScriptPanel = ui.AddRoot<Panel>(0.f, sy, spW, kScriptH, "Script Editor");
        pScriptPanel->visible = false;

        float pad = gStyle.padding;
        float ty  = gStyle.titleH + 2.f;

        std::string scriptPath;
        if (selectedNode) {
            if (auto* script = selectedNode->GetComponent<NucleoScriptComponent>()) {
                scriptPath = resolveProjectFilePath(script->scriptPath);
            }
        }

        std::string scriptLabel = scriptPath.empty()
            ? "Nenhum script associado"
            : fs::path(scriptPath).filename().string();

        pScriptPanel->Add<Label>(pad, ty, scriptLabel.c_str())
                    ->SetColor(gStyle.textAccent.r, gStyle.textAccent.g, gStyle.textAccent.b);

        float rh = kScriptH - ty - gStyle.lineH - pad;
        pScriptEdit = pScriptPanel->Add<RichText>(pad, ty + gStyle.lineH, spW - pad * 2.f, rh);
        pScriptEdit->syntax = RichText::SyntaxMode::None;
        pScriptEdit->SetText("");
        applyEditorTextPreferences(pScriptEdit);

        if (!scriptPath.empty()) {
            auto scriptTab = LightningEditor::BuildAssetTab(scriptPath);
            auto content = LightningEditor::BuildDocumentContent(scriptTab);
            pScriptEdit->syntax = content.syntax;
            pScriptEdit->SetText(content.bodyText);
            applyEditorTextPreferences(pScriptEdit);
        }
    }

    // ── Console panel (dock tab in the bottom tray) ───────────────────────
    std::unique_ptr<Widget> buildConsolePanel()
    {
        auto panel = std::make_unique<Panel>(0.f, 0.f, kW, kScriptH, "");
        pConsolePanel = panel.get();

        float pad = gStyle.padding;
        float lh  = gStyle.lineH;
        float ty  = gStyle.titleH + 2.f;

        // Clear button
        auto* btnClear = pConsolePanel->Add<Button>(kW - pad - 48.f, 4.f, 48.f, lh, "Clear");
        btnClear->onClick = [this]{ Logger::Clear(); };

        // ScrollView for log entries
        float svH = kScriptH - ty - pad;
        pConsoleSV = pConsolePanel->Add<ScrollView>(pad, ty, kW - pad * 2.f, svH);
        pConsoleSV->autoContent = true;

        refreshConsole();
        Logger::ClearDirty();
        return panel;
    }

    void toggleConsole()
    {
        if (!pBottomTrayNode || pBottomTrayNode->panels.empty()) return;
        if ((int)pBottomTrayNode->panels.size() == 1) return;
        pBottomTrayNode->activeIdx = (pBottomTrayNode->activeIdx == 1) ? 0 : 1;
        cacheBottomTrayActiveIdx = pBottomTrayNode->activeIdx;
        noteEngineChange("Container state changed: bottom tray tab");
        pBottomTrayNode->applyPanelGeometry();
    }

    void refreshConsole()
    {
        if (!pConsoleSV) return;
        pConsoleSV->Clear();

        const float lh = gStyle.lineH;
        for (const auto& e : Logger::GetEntries()) {
            auto* lbl = pConsoleSV->Add<Label>(0.f, 0.f, e.text.c_str());
            lbl->h = lh;
            switch (e.level) {
                case Logger::Level::Warning: lbl->SetColor(230, 180, 40);  break;
                case Logger::Level::Error:   lbl->SetColor(220, 70,  70);  break;
                case Logger::Level::Script:  lbl->SetColor(100, 200, 130); break;
                default:                     lbl->SetColor(gStyle.textDim.r, gStyle.textDim.g, gStyle.textDim.b); break;
            }
        }

        // Scroll to bottom
        if (pConsoleSV && !Logger::GetEntries().empty())
            pConsoleSV->scrollOffset = 999999.f;
    }

    // ── Document DockSpace (full workspace for file-sensitive tabs) ─────────
    void buildScriptDock()
    {
        static constexpr float kScriptLeftW = 220.f;
        float fullH = kMainH + kScriptH;

        pScriptDock = ui.AddRoot<DockSpace>(0.f, kTopH, kW, fullH);
        pScriptDock->visible = false;

        DockNode* root = pScriptDock->Root();
        float leftFrac = kScriptLeftW / kW;
        auto [leftNode, editorNode] = root->Split(true, leftFrac);
        auto [filesNode, membersNode] = leftNode->Split(false, 0.5f);

        filesNode->Dock(buildScriptFilesPanel(), "Open Documents");
        membersNode->Dock(buildScriptMembersPanel(), "Document Outline");
        editorNode->Dock(buildScriptEditorPanel(kW - kScriptLeftW, fullH), "Document Workspace");
    }

    std::unique_ptr<Widget> buildScriptFilesPanel()
    {
        auto panel = std::make_unique<Panel>(0.f, 0.f, 200.f, 200.f, "");
        pDocumentFilesPanel = panel.get();
        return panel;
    }

    std::unique_ptr<Widget> buildScriptMembersPanel()
    {
        auto panel = std::make_unique<Panel>(0.f, 0.f, 200.f, 200.f, "");
        pDocumentOutlinePanel = panel.get();
        return panel;
    }

    std::unique_ptr<Widget> buildScriptEditorPanel(float editorW, float editorH)
    {
        auto panel = std::make_unique<Panel>(0.f, 0.f, editorW, editorH, "");
        panel->clipChildren = true;
        pDocumentEditorPanel = panel.get();
        return panel;
    }

    void refreshDocumentWorkspace()
    {
        if (!pDocumentFilesPanel || !pDocumentOutlinePanel || !pDocumentEditorPanel) return;

        LightningEditor::EditorDocumentWorkspaceContext context;
        context.filesPanel = pDocumentFilesPanel;
        context.outlinePanel = pDocumentOutlinePanel;
        context.editorPanel = pDocumentEditorPanel;
        context.tabStrip = pTabStrip;
        context.activeTextEditor = &pScriptDockEdit;
        context.renderer = &renderer;
        context.gridTexture = &gridTex;
        context.previewTexture = &texEditorTex;
        context.previewTexturePath = &texEditorPath;
        context.editorWidth = (pDocumentEditorPanel->w > 0.f) ? pDocumentEditorPanel->w : (kW - 220.f);
        context.editorHeight = (pDocumentEditorPanel->h > 0.f) ? pDocumentEditorPanel->h : (kMainH + kScriptH);
        context.onActivateTab = [this](int idx) { activateTabIndex(idx); };
        context.onTextChanged = [this](const std::vector<std::string>& lines) {
            tabManager.MarkDirty(lines);
            int idx = tabManager.ActiveIndex();
            if (pTabStrip && idx >= 0 && idx < (int)pTabStrip->tabs.size()) {
                const auto* tab = tabManager.ActiveTab();
                if (tab) pTabStrip->tabs[idx].label = tab->DisplayLabel();
            }
        };

        LightningEditor::RenderDocumentWorkspace(tabManager, context);
        applyEditorTextPreferences(pScriptDockEdit);
    }

    // ── Footer ────────────────────────────────────────────────────────────
    void buildFooter()
    {
        float fy = kH - kFootH;
         pFooter = ui.AddRoot<Panel>(0.f, fy, kW, kFootH, "", false);
         pFooter->zOrder = 1;

        float fh  = ui.font.GlyphH();
        float pad = gStyle.padding;
        float ty  = (kFootH - fh) * 0.5f;

         lblFooterLeft = pFooter->Add<Label>(pad, ty, "Lightning Engine Editor v0.4");
         lblFooterLeft->SetColor(gStyle.textDim.r, gStyle.textDim.g, gStyle.textDim.b);

        const char* rLabel = pm.isOpen ? "Project Open" : "Ready";
        float rW = ui.font.MeasureW(rLabel);
         lblFooterRight = pFooter->Add<Label>(kW - rW - pad, ty, rLabel);
         lblFooterRight->SetColor(gStyle.textGreen.r, gStyle.textGreen.g, gStyle.textGreen.b);
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Scene operations (work on real Level / Node)
    // ─────────────────────────────────────────────────────────────────────────

    void addNodeToScene(const std::string& name)
    {
        auto node = std::make_unique<Node>(name);
        Node* raw = node.get();
        editorLevel.AddNode(std::move(node));
        rebuildHierarchyTree();
        noteEngineChange("Node added: " + name);

        // Push undo record (action already done above)
        // Redo = re-add, Undo = remove
        undoStack.Push(
            [this, name](){
                auto n = std::make_unique<Node>(name);
                editorLevel.AddNode(std::move(n));
                rebuildHierarchyTree();
            },
            [this, raw](){
                if (selectedNode == raw) { selectedNode = nullptr; refreshInspector(); }
                editorLevel.RemoveNode(raw);
                rebuildHierarchyTree();
            },
            "Add Node"
        );
    }

    void addChildToSelected()
    {
        if (!selectedNode) { addNodeToScene("Node"); return; }
        auto child = std::make_unique<Node>("Node");
        selectedNode->AddChild(std::move(child));
        rebuildHierarchyTree();
        noteEngineChange("Child node added");
    }

    void duplicateSelected()
    {
        if (!selectedNode) return;
        auto clone = selectedNode->Clone();
        editorLevel.AddNode(std::move(clone));
        rebuildHierarchyTree();
        noteEngineChange("Node duplicated");
    }

    void deleteSelected()
    {
        if (!selectedNode) return;
        std::string deletedName = selectedNode->name;
        // Capture state for undo
        std::string name = selectedNode->name;
        std::string tag  = selectedNode->tag;
        bool active      = selectedNode->active;
        Lightning::V3 pos = selectedNode->transform.Position;
        Lightning::V3 rot = selectedNode->transform.Rotation;
        Lightning::V3 scl = selectedNode->transform.Scale;

        editorLevel.RemoveNode(selectedNode);
        selectedNode = nullptr;
        rebuildHierarchyTree();
        refreshInspector();
        noteEngineChange("Node deleted: " + deletedName);

        // Push undo record (action already done above)
        // Redo = delete again (find by name), Undo = re-create
        undoStack.Push(
            [this, name](){
                for (auto& n : editorLevel.GetNodes()) {
                    if (n->name == name) {
                        if (selectedNode == n.get()) { selectedNode = nullptr; refreshInspector(); }
                        editorLevel.RemoveNode(n.get());
                        break;
                    }
                }
                rebuildHierarchyTree();
            },
            [this, name, tag, active, pos, rot, scl](){
                auto n = std::make_unique<Node>(name);
                n->tag    = tag;
                n->active = active;
                n->transform.Position = pos;
                n->transform.Rotation = rot;
                n->transform.Scale    = scl;
                editorLevel.AddNode(std::move(n));
                rebuildHierarchyTree();
            },
            "Delete Node"
        );
    }

    void newScene()
    {
        if (pm.isOpen) saveScene();
        // Collect pointers first, then remove (avoids iterator invalidation)
        std::vector<Node*> roots;
        for (auto& n : editorLevel.GetNodes()) roots.push_back(n.get());
        for (Node* n : roots) editorLevel.RemoveNode(n);
        selectedNode     = nullptr;
        currentScenePath = "scenes/main.lescene";
        syncPrimarySceneTabLabel();
        rebuildHierarchyTree();
        refreshInspector();
        noteEngineChange("New scene created");
    }

    void saveScene()
    {
        if (!pm.isOpen) return;
        pm.project.lastScene = currentScenePath;
        pm.SaveScene(currentScenePath, editorLevel);
        pm.Save();
        noteEngineChange("Scene saved: " + currentScenePath);
    }

    void saveDirtyDocuments()
    {
        if (tabManager.SaveAllDirty() == 0) return;
        // Refresh tab strip labels to remove dirty indicators
        syncTabStripFromManager();
    }

    void openSceneDialog()
    {
        if (!pm.isOpen) return;

        std::string picked = NativeDialog::PickFileSDL(
            renderer.GetWindow(),
            "Open Scene",
            pm.ScenesDir().c_str());

        if (picked.empty()) return;

        fs::path pickedPath = fs::path(picked);
        if (pickedPath.extension() != ".lescene") {
            Logger::LogWarning("[Editor] Open Scene: unsupported extension (expected .lescene)");
            return;
        }

        std::error_code ec;
        fs::path relPath = fs::relative(pickedPath, fs::path(pm.project.rootPath), ec);
        if (ec || relPath.empty()) {
            Logger::LogWarning("[Editor] Open Scene: path must be inside current project");
            return;
        }

        std::string rel = relPath.generic_string();
        if (rel.rfind("..", 0) == 0) {
            Logger::LogWarning("[Editor] Open Scene: path must be inside current project");
            return;
        }

        if (!pm.LoadScene(rel, editorLevel)) {
            Logger::LogWarning("[Editor] Failed to load scene: " + rel);
            return;
        }

        currentScenePath = rel;
        syncPrimarySceneTabLabel();
        selectedNode = nullptr;
        rebuildHierarchyTree();
        refreshInspector();
        Logger::LogInfo("[Editor] Scene loaded: " + rel);
    }

    void closeAndGoSplash()
    {
        if (pm.isOpen) {
            saveScene();
            pm.Close();
        }
        selectedNode = nullptr;
        isPlaying    = false;
        state        = State::Splash;
        rebuildSplash();
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Chrome / Grid
    // ─────────────────────────────────────────────────────────────────────────

    Texture buildGridTexture()
    {
        static constexpr int kSz = 256, kCell = 128;
        std::vector<Uint32> px(kSz * kSz);
        for (int y = 0; y < kSz; y++)
            for (int x = 0; x < kSz; x++) {
                bool dark = ((x / kCell) + (y / kCell)) % 2 == 0;
                Uint8 v = dark ? 26 : 40;
                px[y * kSz + x] = (255u << 24) | ((Uint32)v << 16) | ((Uint32)v << 8) | v;
            }
        return renderer.LoadTextureFromPixels(px.data(), kSz, kSz, /*nearest=*/true);
    }

    void renderChrome()
    {
        renderer.BeginScreenSpace();

        // Title bar
        renderer.SetDrawColor(16, 16, 22);
        renderer.FillRect(0.f, 0.f, kW, kTitleH);
        {
            const char* pname = pm.isOpen ? pm.project.name.c_str() : "No Project";
            float fh  = ui.font.GlyphH();
            float pnW = ui.font.MeasureW(pname);
            renderer.SetDrawColor(170, 170, 182);
            ui.font.DrawText(renderer, pname, kW - pnW - 10.f, (kTitleH - fh) * 0.5f);
        }

        // MenuBar + Toolbar backgrounds
        renderer.SetDrawColor(24, 24, 32);
        renderer.FillRect(0.f, kTitleH, kW, kMenuH);
        renderer.SetDrawColor(26, 26, 34);
        renderer.FillRect(0.f, kTitleH + kMenuH, kW, kToolH);

        // Separators
        renderer.SetDrawColor(48, 48, 62);
        renderer.FillRect(0.f, kTitleH - 1.f,          kW, 1.f);
        renderer.FillRect(0.f, kTitleH + kMenuH - 1.f, kW, 1.f);
        renderer.FillRect(0.f, kTopH - 1.f,             kW, 1.f);

        if (pDockSpace && pDockSpace->visible) {
            renderer.SetDrawColor(48, 48, 62);
            renderer.FillRect(0.f, kTopH + kMainH, kW, 1.f);
        }

        // Footer
        renderer.SetDrawColor(16, 16, 22);
        renderer.FillRect(0.f, kH - kFootH, kW, kFootH);
        renderer.SetDrawColor(48, 48, 62);
        renderer.FillRect(0.f, kH - kFootH, kW, 1.f);

        renderer.EndScreenSpace();
    }
};
