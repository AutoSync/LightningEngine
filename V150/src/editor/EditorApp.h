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
#include <algorithm>
#include <cctype>
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
#include "../include/gui/TitanUI.h"
#include "../include/gui/TitanStyle.h"
#include "tabs/EditorTabSystem.h"
#include "tabs/EditorDocumentContent.h"

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
    ScrollView*  pCBScroll       = nullptr;
    Panel*       pScriptPanel    = nullptr;
    RichText*    pScriptEdit     = nullptr;
    DockSpace*   pScriptDock     = nullptr;
    Panel*       pDocumentFilesPanel = nullptr;
    Panel*       pDocumentOutlinePanel = nullptr;
    Panel*       pDocumentEditorPanel = nullptr;
    RichText*    pScriptDockEdit = nullptr;
    Panel*       pConsolePanel   = nullptr;
    ScrollView*  pConsoleSV      = nullptr;
    bool         consoleVisible  = true;
    int          newProjTemplate = 0;   // 0=Empty 1=2D 2=3D
    ContextMenu* hierMenu        = nullptr;
    ContextMenu* cbMenu          = nullptr;

    std::string  cbCurrentDir;

    Texture logoTex;
    Texture splashTex;
    Texture gridTex;

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

    void openContentDirectory(const fs::path& dir)
    {
        if (!pm.isOpen) return;
        std::error_code ec;
        if (!fs::exists(dir, ec) || !fs::is_directory(dir, ec)) return;

        fs::path root = contentRootDir();
        fs::path rel  = fs::relative(dir, root, ec);
        if (ec || rel.string().rfind("..", 0) == 0) return;

        cbCurrentDir = dir.string();
        refreshContentBrowser();
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
        refreshContentBrowser();
        openAssetContextTab(target.string());
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
        logoTex   = renderer.LoadTexture("assets/icons/logo.png");
        splashTex = renderer.LoadTexture("assets/splash/splashscreen.png");
        gridTex   = buildGridTexture();

        kW = (float)GetWidth();
        kH = (float)GetHeight();
        computeLayout();

        // Inject context into the live level (no nodes yet)
        editorLevel.SetContext(renderer, inputManager);
        editorLevel.Initialize();

        buildSplash();
    }

    void Shutdown() override
    {
        editorLevel.Shutdown();
        if (pm.isOpen) pm.Save();
        ui.Release();
        logoTex.Release();
        splashTex.Release();
        gridTex.Release();
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
                computeLayout();
                if (state == State::Editor) {
                    if (pDockSpace) {
                        pDockSpace->Resize(0.f, kTopH, kW, kMainH + kScriptH);
                        if (pScriptDock)
                            pScriptDock->Resize(0.f, kTopH, kW, kMainH + kScriptH);
                        if (pTabStrip)    pTabStrip->w    = kW - kLogoW - kProjNameW;
                        if (pMenuBar)     pMenuBar->w     = kW;
                        if (pToolbar)     pToolbar->w     = kW;
                    } else {
                        rebuildEditorUI();
                    }
                } else {
                    rebuildSplash();
                }
            }
        }

        ui.ProcessInput(inputManager);

        if (state == State::Editor) {
            // Ctrl+Z / Ctrl+Y — undo/redo
            bool ctrl = inputManager.IsKeyDown(SDL_SCANCODE_LCTRL) ||
                        inputManager.IsKeyDown(SDL_SCANCODE_RCTRL);
            if (ctrl && inputManager.IsKeyPressed(SDL_SCANCODE_Z)) undoStack.Undo();
            if (ctrl && inputManager.IsKeyPressed(SDL_SCANCODE_Y)) undoStack.Redo();
        }

        if (state == State::Editor) {
            // Hierarchy right-click context menu
            if (inputManager.IsMousePressed(3)) {
                float mx = inputManager.GetMouseX();
                float my = inputManager.GetMouseY();
                bool overCB = pContentBrow && pContentBrow->Contains(mx, my, 0.f, 0.f);
                bool overHier = pHierarchy && pHierarchy->Contains(mx, my, 0.f, 0.f);
                if (overCB && cbMenu) cbMenu->Open(mx, my);
                else if (overHier && hierMenu) hierMenu->Open(mx, my);
            }

            // Play: tick real scene (scripts run here)
            if (isPlaying) editorLevel.Update(dt);

            // Game Preview window tick
            if (gamePreview.IsOpen()) gamePreview.Tick(dt);

            // Gizmo drag
            if (!isPlaying && selectedNode)
                processGizmoDrag();

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

        // ── Background: full-screen splash image ──────────────────────────
        pSplash = ui.AddRoot<Panel>(0.f, 0.f, kW, kH, "", false);
        pSplash->zOrder = 0;
        if (splashTex.IsValid())
            pSplash->Add<Image>(0.f, 0.f, kW, kH, &splashTex);

        // ── Right welcome panel ────────────────────────────────────────────
        static constexpr float kPanelW = 370.f;
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

                    // Small logo icon
                    if (logoTex.IsValid())
                        panel->Add<Image>(pad, ty + (entryH - 14.f) * 0.5f,
                                          14.f, 14.f, &logoTex);

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
        ui.ClearRoots();
        clearEditorPtrs();
        resetTabsToSceneOnly();
        buildEditorUI();
        rebuildHierarchyTree();
        refreshContentBrowser();
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
        int   selTabIdx = tabManager.ActiveIndex();
        ui.ClearRoots();
        clearEditorPtrs();
        tabManager.Activate(selTabIdx);
        buildEditorUI();
        rebuildHierarchyTree();
        refreshContentBrowser();
    }

    void clearEditorPtrs()
    {
        pSplash = nullptr; pNewProjModal = nullptr; pOpenProjModal = nullptr;
        tfNewName = nullptr; tfNewPath = nullptr; tfOpenPath = nullptr;
        lblNewStatus = nullptr; lblOpenStatus = nullptr;
        pTabStrip = nullptr; pMenuBar = nullptr; pToolbar = nullptr;
        pDockSpace = nullptr; pBottomTrayNode = nullptr; pCamWidget = nullptr; pContentBrow = nullptr;
        pHierarchy = nullptr; pInspector = nullptr; inspLastW = 0.f;
        pViewport = nullptr; pHierTree = nullptr; pCBScroll = nullptr;
        pScriptPanel = nullptr; pScriptEdit = nullptr;
        pScriptDock = nullptr; pDocumentFilesPanel = nullptr;
        pDocumentOutlinePanel = nullptr; pDocumentEditorPanel = nullptr;
        pScriptDockEdit = nullptr;
        pConsolePanel = nullptr; pConsoleSV = nullptr;
        hierMenu = nullptr; cbMenu = nullptr; lblFps = nullptr;
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
        pMenuBar->AddItem("TOOLS", "Open Equinox",      []{});
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
        bottomNode->activeIdx = 0;
        bottomNode->applyPanelGeometry();

        left->SetLayout(pHierarchy, [this](float w, float h) {
            float pad = gStyle.padding;
            if (pHierTree) { pHierTree->w = w - pad * 2.f; pHierTree->h = h - pHierTree->y - pad; }
        });
        bottomNode->SetLayout(pContentBrow, [this](float w, float h) {
            float pad = gStyle.padding;
            if (pCBScroll) { pCBScroll->w = w - pad * 2.f; pCBScroll->h = h - pCBScroll->y - pad; }
        });
        bottomNode->SetLayout(pConsolePanel, [this](float w, float h) {
            float pad = gStyle.padding;
            if (pConsoleSV) { pConsoleSV->w = w - pad * 2.f; pConsoleSV->h = h - pConsoleSV->y - pad; }
        });
        right->SetLayout(pInspector, [this](float w, float /*h*/) {
            if (std::abs(w - inspLastW) > 0.5f) { inspLastW = w; refreshInspector(); }
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
        pContentBrow = panel.get();
        refreshContentBrowser();
        return panel;
    }

    void refreshContentBrowser()
    {
        if (!pContentBrow) return;
        pContentBrow->Clear();

        const float pad = gStyle.padding;
        const float lh  = gStyle.lineH;
        const float panelW = (pContentBrow->w > 0.f) ? pContentBrow->w : kW;
        const float panelH = (pContentBrow->h > 0.f) ? pContentBrow->h : kScriptH;
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
        ty += lh + 6.f;

        if (!pm.isOpen) {
            auto* e = pContentBrow->Add<Label>(pad, ty, "(no project)");
            e->h = lh;
            e->SetColor(gStyle.textDim.r, gStyle.textDim.g, gStyle.textDim.b);
            pCBScroll = nullptr;
            return;
        }

        ensureCBDirValid();
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

        float svH = panelH - ty - pad;
        pCBScroll = pContentBrow->Add<ScrollView>(pad, ty, panelW - pad * 2.f, svH);
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

        auto addDir = [this](const fs::path& absDir) {
            float rowH = gStyle.lineH;
            float contentW = (pCBScroll->w > 0.f) ? pCBScroll->w : (kW - gStyle.padding * 2.f);
            std::string label = "[DIR] " + absDir.filename().string();
            auto* btn = pCBScroll->Add<Button>(0.f, 0.f, contentW, rowH, label.c_str());
            btn->h = rowH;
            btn->SetColor(125, 170, 255);
            btn->onClick = [this, absDir] { openContentDirectory(absDir); };
        };

        auto addFile = [this](const fs::path& absFile) {
            float rowH = gStyle.lineH;
            float contentW = (pCBScroll->w > 0.f) ? pCBScroll->w : (kW - gStyle.padding * 2.f);
            std::string ext = absFile.extension().string();
            auto* btn = pCBScroll->Add<Button>(0.f, 0.f, contentW, rowH, absFile.filename().string().c_str());
            btn->h = rowH;

            std::string lowerExt = LightningEditor::ToLowerCopy(ext);
            if (lowerExt == ".lescene") {
                btn->SetColor(75, 195, 75);
            } else {
                auto tab = LightningEditor::BuildAssetTab(absFile.string());
                btn->SetColor(tab.accent.r, tab.accent.g, tab.accent.b);
            }

            btn->onClick = [this, absFile, ext] {
                cbDragFile = absFile.string();
                cbDragExt  = ext;
                cbDragging = true;
                openAssetContextTab(absFile.string());
            };
        };

        for (const auto& d : dirs) addDir(d.path());
        for (const auto& f : files) addFile(f.path());

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

            // Grid background
            if (gridTex.IsValid()) {
                static constexpr float kTile = 256.f;
                r.SetDrawColor(255, 255, 255);
                for (float ty = ay; ty < ay + vh; ty += kTile)
                    for (float tx = ax; tx < ax + vw; tx += kTile) {
                        float tw = std::min(kTile, ax + vw - tx);
                        float th = std::min(kTile, ay + vh - ty);
                        r.DrawTextureRegion(gridTex, tx, ty, tw, th,
                                            0.f, 0.f, tw / kTile, th / kTile);
                    }
            } else {
                r.SetDrawColor(20, 20, 26);
                r.FillRect(ax, ay, vw, vh);
            }

            // Render real scene nodes (positions are in viewport-local screen coords)
            r.EndScreenSpace();
            editorLevel.Render();    // node components draw via renderer (screen-space coords)
            r.BeginScreenSpace();

            // Gizmos (move handles) — only in editor mode
            if (selectedNode && !isPlaying) {
                static constexpr float kArrow = 50.f;
                static constexpr float kTip   = 5.f;
                auto wp = selectedNode->WorldPosition();
                float nx = ax + wp.x, ny = ay + wp.y;

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
    void buildContextMenus()
    {
        hierMenu = ui.AddRoot<ContextMenu>();
        hierMenu->AddItem("Add Child Node",  [this]{ addChildToSelected(); });
        hierMenu->AddItem("Duplicate",       [this]{ duplicateSelected(); });
        hierMenu->AddSeparator();
        hierMenu->AddItem("Delete",          [this]{ deleteSelected(); });
        hierMenu->AddSeparator();
        hierMenu->AddItem("Add Script",      [this]{
            if (selectedNode && !selectedNode->HasComponent<NucleoScriptComponent>()) {
                selectedNode->AddComponent<NucleoScriptComponent>();
                refreshInspector();
            }
        });
        ui.BringToFront(hierMenu);

        cbMenu = ui.AddRoot<ContextMenu>();
        cbMenu->AddItem("Importar arquivo aqui", [this]{ importFileToCurrentDir(); });
        cbMenu->AddItem("Nova pasta", [this]{ createFolderInCurrentDir(); });
        cbMenu->AddSeparator();
        cbMenu->AddItem("Novo Script (.spark)", [this]{
            createAssetInCurrentDir("NewScript", ".spark",
                "class NewScript {\n"
                "    void OnStart() {}\n"
                "    void Update(float dt) {}\n"
                "}\n");
        });
        cbMenu->AddItem("Novo Shader Vert (.vert)", [this]{
            createAssetInCurrentDir("new_shader", ".vert",
                "#version 450\n\n"
                "layout(location = 0) in vec3 inPos;\n"
                "void main() { gl_Position = vec4(inPos, 1.0); }\n");
        });
        cbMenu->AddItem("Novo Shader Frag (.frag)", [this]{
            createAssetInCurrentDir("new_shader", ".frag",
                "#version 450\n\n"
                "layout(location = 0) out vec4 outColor;\n"
                "void main() { outColor = vec4(1.0); }\n");
        });
        cbMenu->AddItem("Novo Prefab (.lprefab)", [this]{
            createAssetInCurrentDir("NewPrefab", ".lprefab",
                "PREFAB \"NewPrefab\"\n"
                "NODE Node \"Root\"\n"
                "  TRANSFORM 0 0 0  0 0 0  1 1 1\n"
                "  TAG \"\"\n"
                "  ACTIVE 1\n"
                "END\n");
        });
        cbMenu->AddItem("Novo Level (.lescene)", [this]{
            createAssetInCurrentDir("NewScene", ".lescene",
                "# Lightning Engine Scene 1.0\n");
        });
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
                node->transform.Position = Lightning::V3(dropX - vpAX, dropY - vpAY, 0.f);
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
        float nx = vpAX + wp.x;
        float ny = vpAY + wp.y;

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
            float dx = mx - gizmoDragMX0;
            float dy = my - gizmoDragMY0;
            if (gizmoAxis == GizmoAxis::X || gizmoAxis == GizmoAxis::XY)
                selectedNode->transform.Position.x = gizmoDragNX0 + dx;
            if (gizmoAxis == GizmoAxis::Y || gizmoAxis == GizmoAxis::XY)
                selectedNode->transform.Position.y = gizmoDragNY0 + dy;
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

        if (!scriptPath.empty()) {
            auto scriptTab = LightningEditor::BuildAssetTab(scriptPath);
            auto content = LightningEditor::BuildDocumentContent(scriptTab);
            pScriptEdit->syntax = content.syntax;
            pScriptEdit->SetText(content.bodyText);
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
        pDocumentEditorPanel = panel.get();
        return panel;
    }

    void refreshDocumentWorkspace()
    {
        if (!pDocumentFilesPanel || !pDocumentOutlinePanel || !pDocumentEditorPanel) return;

        const float pad = gStyle.padding;
        const float lh = gStyle.lineH;
        pScriptDockEdit = nullptr;

        pDocumentFilesPanel->Clear();
        pDocumentOutlinePanel->Clear();
        pDocumentEditorPanel->Clear();

        float filesW = (pDocumentFilesPanel->w > 0.f) ? pDocumentFilesPanel->w : 200.f;
        float editorW = (pDocumentEditorPanel->w > 0.f) ? pDocumentEditorPanel->w : (kW - 220.f);
        float editorH = (pDocumentEditorPanel->h > 0.f) ? pDocumentEditorPanel->h : (kMainH + kScriptH);

        float ty = pad;
        pDocumentFilesPanel->Add<Label>(pad, ty, "Abas abertas")
            ->SetColor(gStyle.textBright.r, gStyle.textBright.g, gStyle.textBright.b);
        ty += lh + 6.f;

        int openDocs = 0;
        const auto& tabs = tabManager.Tabs();
        for (int i = 0; i < (int)tabs.size(); ++i) {
            const auto& tab = tabs[i];
            if (!tab.IsDocumentWorkspace()) continue;
            ++openDocs;
            auto* btn = pDocumentFilesPanel->Add<Button>(pad, ty, filesW - pad * 2.f, lh + 2.f, tab.label.c_str());
            btn->SetColor(tab.accent.r, tab.accent.g, tab.accent.b);
            btn->onClick = [this, i] { activateTabIndex(i); };
            ty += lh + 4.f;
        }

        if (openDocs == 0) {
            auto* empty = pDocumentFilesPanel->Add<Label>(pad, ty, "Nenhum documento aberto.");
            empty->h = lh;
            empty->SetColor(gStyle.textDim.r, gStyle.textDim.g, gStyle.textDim.b);
        }

        const auto* active = tabManager.ActiveTab();
        if (!active || !active->IsDocumentWorkspace()) {
            auto* idle = pDocumentOutlinePanel->Add<Label>(pad, pad, "Abra um asset para gerar o workspace contextual.");
            idle->h = lh;
            idle->SetColor(gStyle.textDim.r, gStyle.textDim.g, gStyle.textDim.b);
            auto* idleEditor = pDocumentEditorPanel->Add<Label>(pad, pad, "O conteudo do documento aparece aqui.");
            idleEditor->h = lh;
            idleEditor->SetColor(gStyle.textDim.r, gStyle.textDim.g, gStyle.textDim.b);
            return;
        }

        LightningEditor::EditorDocumentContent content = LightningEditor::BuildDocumentContent(*active);

        float infoY = pad;
        pDocumentOutlinePanel->Add<Label>(pad, infoY, content.kindLabel.c_str())
            ->SetColor(active->accent.r, active->accent.g, active->accent.b);
        infoY += lh + 4.f;

        auto* pathLbl = pDocumentOutlinePanel->Add<Label>(pad, infoY, content.editorTitle.c_str());
        pathLbl->h = lh;
        pathLbl->SetColor(gStyle.textBright.r, gStyle.textBright.g, gStyle.textBright.b);
        infoY += lh + 2.f;

        auto* statusLbl = pDocumentOutlinePanel->Add<Label>(pad, infoY, content.statusLabel.c_str());
        statusLbl->h = lh * 2.f;
        statusLbl->SetColor(gStyle.textDim.r, gStyle.textDim.g, gStyle.textDim.b);
        infoY += lh + 8.f;

        auto* outlineTitle = pDocumentOutlinePanel->Add<Label>(pad, infoY, "Estrutura derivada");
        outlineTitle->h = lh;
        outlineTitle->SetColor(gStyle.textBright.r, gStyle.textBright.g, gStyle.textBright.b);
        infoY += lh + 4.f;

        if (content.outline.empty()) {
            auto* empty = pDocumentOutlinePanel->Add<Label>(pad, infoY, "Sem estrutura derivada disponivel.");
            empty->h = lh;
            empty->SetColor(gStyle.textDim.r, gStyle.textDim.g, gStyle.textDim.b);
        } else {
            for (const auto& item : content.outline) {
                auto* lbl = pDocumentOutlinePanel->Add<Label>(pad, infoY, item.label.c_str());
                lbl->h = lh;
                lbl->SetColor(item.r, item.g, item.b);
                infoY += lh + 2.f;
            }
        }

        float editorY = pad;
        pDocumentEditorPanel->Add<Label>(pad, editorY, content.editorTitle.c_str())
            ->SetColor(active->accent.r, active->accent.g, active->accent.b);
        editorY += lh;

        auto* fileLbl = pDocumentEditorPanel->Add<Label>(pad, editorY, content.pathLabel.c_str());
        fileLbl->h = lh;
        fileLbl->SetColor(gStyle.textDim.r, gStyle.textDim.g, gStyle.textDim.b);
        editorY += lh + 4.f;

        if (content.textual) {
            float rh = editorH - editorY - pad;
            pScriptDockEdit = pDocumentEditorPanel->Add<RichText>(pad, editorY, editorW - pad * 2.f, rh);
            pScriptDockEdit->syntax = content.syntax;
            pScriptDockEdit->SetText(content.bodyText);
            pScriptDockEdit->onChanged = [this](const std::vector<std::string>& lines) {
                tabManager.MarkDirty(lines);
                // Reflect dirty indicator on tab strip immediately
                int idx = tabManager.ActiveIndex();
                if (pTabStrip && idx >= 0 && idx < (int)pTabStrip->tabs.size()) {
                    const auto* tab = tabManager.ActiveTab();
                    if (tab) pTabStrip->tabs[idx].label = tab->DisplayLabel();
                }
            };
            return;
        }

        auto* info1 = pDocumentEditorPanel->Add<Label>(pad, editorY, "Este tipo de arquivo usa fluxo de importacao ou editor dedicado.");
        info1->h = lh;
        info1->SetColor(gStyle.textBright.r, gStyle.textBright.g, gStyle.textBright.b);
        editorY += lh + 4.f;

        auto* info2 = pDocumentEditorPanel->Add<Label>(pad, editorY, "A aba organiza o contexto pelo arquivo aberto, sem conteudo de exemplo embutido.");
        info2->h = lh;
        info2->SetColor(gStyle.textDim.r, gStyle.textDim.g, gStyle.textDim.b);
    }

    // ── Footer ────────────────────────────────────────────────────────────
    void buildFooter()
    {
        float fy = kH - kFootH;
        auto* footer = ui.AddRoot<Panel>(0.f, fy, kW, kFootH, "", false);
        footer->zOrder = 1;

        float fh  = ui.font.GlyphH();
        float pad = gStyle.padding;
        float ty  = (kFootH - fh) * 0.5f;

        footer->Add<Label>(pad, ty, "Lightning Engine Editor v0.4")
               ->SetColor(gStyle.textDim.r, gStyle.textDim.g, gStyle.textDim.b);

        const char* rLabel = pm.isOpen ? "Project Open" : "Ready";
        float rW = ui.font.MeasureW(rLabel);
        footer->Add<Label>(kW - rW - pad, ty, rLabel)
               ->SetColor(gStyle.textGreen.r, gStyle.textGreen.g, gStyle.textGreen.b);
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
    }

    void duplicateSelected()
    {
        if (!selectedNode) return;
        auto clone = selectedNode->Clone();
        editorLevel.AddNode(std::move(clone));
        rebuildHierarchyTree();
    }

    void deleteSelected()
    {
        if (!selectedNode) return;
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
    }

    void saveScene()
    {
        if (!pm.isOpen) return;
        pm.project.lastScene = currentScenePath;
        pm.SaveScene(currentScenePath, editorLevel);
        pm.Save();
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
