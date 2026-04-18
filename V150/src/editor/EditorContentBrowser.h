// EditorContentBrowser.h — Content Browser panel: file listing, icons, drag & drop.
#pragma once
#include "EditorPanel.h"
#include "../include/NativeDialog.h"
#include "../include/Equinox.h"
#include "../include/Nucleo.h"
#include <fstream>

class EditorContentBrowser : public EditorPanel
{
public:
    explicit EditorContentBrowser(EditorContext& ctx) : ctx(ctx) {}

    // ── UI pointers ───────────────────────────────────────────────────────
    Panel*       pContentBrow    = nullptr;
    TreeView*    pCBTree         = nullptr;
    ScrollView*  pCBScroll       = nullptr;
    Grid*        pCBGrid         = nullptr;
    ContextMenu* cbMenu          = nullptr;
    Panel*       pCBRenameModal  = nullptr;
    TextField*   tfCBRename      = nullptr;
    Label*       lblCBRenameStat = nullptr;
    Panel*       pCBPropsModal   = nullptr;

    // ── Selection / navigation state ──────────────────────────────────────
    std::string  cbCurrentDir;
    std::string  cbSelectedPath;
    bool         cbSelectedIsDir  = false;
    std::string  cbLastClickPath;
    Uint64       cbLastClickMs    = 0;
    std::string  cbRenameTargetPath;
    std::string  cbRenameTargetExt;
    std::vector<std::pair<TreeNode*, std::string>> cbTreeNodePaths;
    int          cbTypeFilter     = 0;
    int          cbViewMode       = 0;
    float        cbLastPanelW     = 0.f;
    float        cbLastPanelH     = 0.f;

    // ── Drag & drop ───────────────────────────────────────────────────────
    bool         cbDragging       = false;
    std::string  cbDragFile;
    std::string  cbDragExt;

    // ── Plugin wizard ─────────────────────────────────────────────────────
    Panel*       pPluginWizardModal  = nullptr;
    TextField*   tfPluginName        = nullptr;
    TextField*   tfPluginId          = nullptr;
    TextField*   tfPluginVersion     = nullptr;
    TextField*   tfPluginCategory    = nullptr;
    TextField*   tfPluginSubcat      = nullptr;
    Label*       lblPluginWizardStat = nullptr;
    int          pluginScopeIdx      = 0;
    int          pluginTypeIdx       = 0;

    // ── Icons ─────────────────────────────────────────────────────────────
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

    // ── EditorPanel interface ─────────────────────────────────────────────

    std::unique_ptr<Widget> Build() override
    {
        auto panel = std::make_unique<Panel>(0.f, 0.f, 400.f, 180.f, "");
        panel->clipChildren = true;
        pContentBrow = panel.get();
        Refresh();
        return panel;
    }

    void Refresh() override
    {
        if (!pContentBrow) return;
        pContentBrow->Clear();
        pCBTree = nullptr;
        pCBScroll = nullptr;
        pCBGrid = nullptr;
        cbTreeNodePaths.clear();

        const float pad    = gStyle.padding;
        const float lh     = gStyle.lineH;
        const float panelW = (pContentBrow->w > 0.f) ? pContentBrow->w : 400.f;
        const float panelH = (pContentBrow->h > 0.f) ? pContentBrow->h : 180.f;
        cbLastPanelW = panelW;
        cbLastPanelH = panelH;
        float ty = pad;

        pContentBrow->Add<Label>(pad, ty, "Content Browser")
            ->SetColor(gStyle.textBright.r, gStyle.textBright.g, gStyle.textBright.b);

        float btnW = 70.f;
        auto* btnUp = pContentBrow->Add<Button>(panelW - pad - btnW, ty - 1.f, btnW, lh + 2.f, "Subir");
        btnUp->onClick = [this]{ goParent(); };
        ty += lh + 4.f;

        auto* btnRefresh = pContentBrow->Add<Button>(pad, ty, 80.f, lh + 2.f, "Atualizar");
        btnRefresh->onClick = [this]{ Refresh(); };
        auto* btnImport = pContentBrow->Add<Button>(pad + 84.f, ty, 80.f, lh + 2.f, "Importar");
        btnImport->onClick = [this]{ importFileToCurrentDir(); };
        auto* btnFolder = pContentBrow->Add<Button>(pad + 168.f, ty, 96.f, lh + 2.f, "Nova Pasta");
        btnFolder->onClick = [this]{ createFolderInCurrentDir(); };

        auto* ddType = pContentBrow->Add<Dropdown>(panelW - pad - btnW - 228.f, ty, 120.f, lh + 2.f, "Tipo");
        ddType->AddItem("All");   ddType->AddItem("Images");
        ddType->AddItem("Scripts"); ddType->AddItem("Scenes");
        ddType->AddItem("Shaders"); ddType->AddItem("Config");
        ddType->SetSelected(cbTypeFilter);
        ddType->onSelect = [this](int idx, const std::string&) {
            cbTypeFilter = idx;
            Refresh();
            if (ctx.noteChange) ctx.noteChange("Content browser type filter changed");
        };

        auto* ddView = pContentBrow->Add<Dropdown>(panelW - pad - btnW - 102.f, ty, 96.f, lh + 2.f, "View");
        ddView->AddItem("Grid"); ddView->AddItem("List");
        ddView->SetSelected(cbViewMode);
        ddView->onSelect = [this](int idx, const std::string&) {
            cbViewMode = idx;
            Refresh();
            if (ctx.noteChange) ctx.noteChange("Content browser view mode changed");
        };
        ty += lh + 6.f;

        if (!ctx.pm.isOpen) {
            auto* e = pContentBrow->Add<Label>(pad, ty, "(no project)");
            e->h = lh;
            e->SetColor(gStyle.textDim.r, gStyle.textDim.g, gStyle.textDim.b);
            return;
        }

        ensureDirValid();
        if (!cbSelectedPath.empty()) {
            std::error_code sec;
            if (!fs::exists(fs::path(cbSelectedPath), sec)) {
                cbSelectedPath.clear();
                cbLastClickPath.clear();
            }
        }

        fs::path root = ctx.contentRootDir();
        fs::path cur(cbCurrentDir);
        std::error_code ec;
        std::string crumb = "Root";
        fs::path rel = fs::relative(cur, root, ec);
        if (!ec && !rel.empty() && rel.string() != ".")
            for (const auto& part : rel) crumb += " > " + part.string();

        auto* pathLbl = pContentBrow->Add<Label>(pad, ty, crumb.c_str());
        pathLbl->h = lh;
        pathLbl->SetColor(gStyle.textDim.r, gStyle.textDim.g, gStyle.textDim.b);
        ty += lh + 4.f;

        float bodyH   = panelH - ty - pad;
        float treeW   = std::max(170.f, std::min(300.f, panelW * 0.26f));
        float splitGap= 6.f;
        float rightX  = pad + treeW + splitGap;
        float rightW  = panelW - rightX - pad;

        pCBTree = pContentBrow->Add<TreeView>(pad, ty, treeW, bodyH);
        pCBTree->showRoot = true;
        pCBTree->itemH = lh + 2.f;
        pCBTree->root.label = fs::path(ctx.pm.project.rootPath).filename().string();
        if (pCBTree->root.label.empty()) pCBTree->root.label = "Project";
        pCBTree->root.expanded = true;
        cbTreeNodePaths.push_back({ &pCBTree->root, root.string() });
        buildTreeRecursive(&pCBTree->root, root);
        pCBTree->onSelect = [this](TreeNode* tn) {
            std::string path = treePathForNode(tn);
            if (!path.empty()) openDirectory(fs::path(path));
        };

        pCBScroll = pContentBrow->Add<ScrollView>(rightX, ty, rightW, bodyH);
        pCBScroll->autoContent = true;

        std::vector<fs::directory_entry> dirs, files;
        try {
            for (auto& entry : fs::directory_iterator(cur)) {
                if (entry.is_directory()) dirs.push_back(entry);
                else if (entry.is_regular_file()) files.push_back(entry);
            }
        } catch (...) {}

        auto byName = [](const fs::directory_entry& a, const fs::directory_entry& b) {
            return LightningEditor::ToLowerCopy(a.path().filename().string()) <
                   LightningEditor::ToLowerCopy(b.path().filename().string());
        };
        std::sort(dirs.begin(), dirs.end(), byName);
        std::sort(files.begin(), files.end(), byName);

        if (cbViewMode == 0) buildGridView(dirs, files, rightW);
        else                  buildListView(dirs, files, rightW);
    }

    void ClearPtrs() override
    {
        pContentBrow = nullptr;
        pCBTree = nullptr;
        pCBScroll = nullptr;
        pCBGrid = nullptr;
        cbMenu = nullptr;
        pCBRenameModal = nullptr;
        tfCBRename = nullptr;
        lblCBRenameStat = nullptr;
        pCBPropsModal = nullptr;
        pPluginWizardModal = nullptr;
    }

    void LoadIcons(Renderer& r)
    {
        auto load = [&](Texture& tex, const std::string& name) {
            std::string path = "assets/thumbnails/" + name + ".png";
            if (fs::exists(path)) tex.Load(r, path);
        };
        load(cbIconDirThumb,        "folder");
        load(cbIconDirCppThumb,     "folder_cpp");
        load(cbIconDirPluginsThumb, "folder_plugins");
        load(cbIconFolder,          "folder_small");
        load(cbIconDefault,         "file_default");
        load(cbIconTexture,         "file_texture");
        load(cbIconScript,          "file_script");
        load(cbIconScene,           "file_scene");
        load(cbIconShader,          "file_shader");
        load(cbIconConfig,          "file_config");
        load(cbIconMaterial,        "file_material");
        load(cbIconStaticMesh,      "file_mesh");
        load(cbIconParticles,       "file_particles");
    }

    // ── Public helpers used by EditorApp ───────────────────────────────────

    void openDirectory(const fs::path& dir)
    {
        if (!ctx.pm.isOpen) return;
        std::error_code ec;
        if (!fs::exists(dir, ec) || !fs::is_directory(dir, ec)) return;
        fs::path root = ctx.contentRootDir();
        fs::path rel  = fs::relative(dir, root, ec);
        if (ec || rel.string().rfind("..", 0) == 0) return;

        cbCurrentDir = dir.string();
        cbSelectedPath.clear();
        cbLastClickPath.clear();
        cbDragging = false; cbDragFile.clear(); cbDragExt.clear();
        if (ctx.noteChange) ctx.noteChange("Content browser directory changed");
        Refresh();
    }

    void handleEntryClick(const fs::path& absPath, bool isDirectory)
    {
        const std::string clicked = absPath.string();
        const Uint64 now = SDL_GetTicks();
        const bool isDouble = (clicked == cbLastClickPath) && (now - cbLastClickMs <= 350ULL);

        cbSelectedPath = clicked;
        cbSelectedIsDir = isDirectory;
        cbLastClickPath = clicked;
        cbLastClickMs = now;
        if (ctx.noteChange) ctx.noteChange("Content browser selection changed");
        if (!isDouble) return;
        if (isDirectory) openDirectory(absPath);
        else if (ctx.openAssetTab) ctx.openAssetTab(clicked);
    }

    bool matchesTypeFilter(const fs::path& absFile) const
    {
        if (cbTypeFilter == 0) return true;
        std::string ext = LightningEditor::ToLowerCopy(absFile.extension().string());
        switch (cbTypeFilter) {
            case 1: return ext==".png"||ext==".jpg"||ext==".jpeg"||ext==".bmp"||ext==".tga";
            case 2: return ext==".spark"||ext==".cs";
            case 3: return ext==".lescene"||ext==".lprefab"||ext==".prefab";
            case 4: return ext==".vert"||ext==".frag"||ext==".spv"||ext==".spark";
            case 5: return ext==".ini"||ext==".json"||ext==".yaml"||ext==".yml"||ext==".toml";
            default: return true;
        }
    }

    Texture* resolveDirIcon(const fs::path& absDir)
    {
        std::string name = LightningEditor::ToLowerCopy(absDir.filename().string());
        if (name.find("script")!=std::string::npos && cbIconDirCppThumb.IsValid()) return &cbIconDirCppThumb;
        if (name.find("plugin")!=std::string::npos && cbIconDirPluginsThumb.IsValid()) return &cbIconDirPluginsThumb;
        if (cbIconDirThumb.IsValid()) return &cbIconDirThumb;
        if (cbIconFolder.IsValid())   return &cbIconFolder;
        if (cbIconDefault.IsValid())  return &cbIconDefault;
        return nullptr;
    }

    Texture* resolveFileIcon(const fs::path& absFile)
    {
        auto tab = LightningEditor::BuildAssetTab(absFile.string());
        switch (tab.kind) {
            case LightningEditor::EditorTabKind::Texture:      if (cbIconTexture.IsValid())    return &cbIconTexture; break;
            case LightningEditor::EditorTabKind::Script:       if (cbIconScript.IsValid())     return &cbIconScript; break;
            case LightningEditor::EditorTabKind::Scene:
            case LightningEditor::EditorTabKind::Prefab:       if (cbIconScene.IsValid())      return &cbIconScene; break;
            case LightningEditor::EditorTabKind::Shader:       if (cbIconShader.IsValid())     return &cbIconShader; break;
            case LightningEditor::EditorTabKind::Config:       if (cbIconConfig.IsValid())     return &cbIconConfig; break;
            case LightningEditor::EditorTabKind::Material:     if (cbIconMaterial.IsValid())   return &cbIconMaterial; break;
            case LightningEditor::EditorTabKind::StaticMesh:
            case LightningEditor::EditorTabKind::SkeletalMesh: if (cbIconStaticMesh.IsValid()) return &cbIconStaticMesh; break;
            case LightningEditor::EditorTabKind::Particle:     if (cbIconParticles.IsValid())  return &cbIconParticles; break;
            default: break;
        }
        if (cbIconDefault.IsValid()) return &cbIconDefault;
        return nullptr;
    }

    void importFileToCurrentDir()
    {
        if (!ctx.pm.isOpen || !ensureDirValid()) return;
        std::string picked = NativeDialog::PickFileSDL(ctx.renderer.GetWindow(), "Import Asset", cbCurrentDir.c_str());
        if (picked.empty()) return;
        std::error_code ec;
        fs::path src(picked);
        if (!fs::exists(src, ec) || !fs::is_regular_file(src, ec)) return;

        fs::path dst = ctx.makeUniquePath(fs::path(cbCurrentDir), src.stem().string(), src.extension().string());
        fs::copy_file(src, dst, fs::copy_options::none, ec);
        if (ec) { Logger::LogWarning("[Editor] Import failed: " + ec.message()); return; }
        Logger::LogInfo("[Editor] Imported: " + dst.filename().string());
        if (ctx.noteChange) ctx.noteChange("Asset imported: " + dst.filename().string());
        Refresh();
        if (ctx.openAssetTab) ctx.openAssetTab(dst.string());
    }

    void createFolderInCurrentDir()
    {
        if (!ctx.pm.isOpen || !ensureDirValid()) return;
        std::error_code ec;
        fs::path target = ctx.makeUniquePath(fs::path(cbCurrentDir), "NewFolder", "");
        fs::create_directories(target, ec);
        if (ec) { Logger::LogWarning("[Editor] Failed to create folder: " + ec.message()); return; }
        if (ctx.noteChange) ctx.noteChange("Folder created: " + target.filename().string());
        Refresh();
    }

    void createAssetInCurrentDir(const std::string& stem, const std::string& ext, const std::string& initial = "")
    {
        if (!ctx.pm.isOpen || !ensureDirValid()) return;
        fs::path target = ctx.makeUniquePath(fs::path(cbCurrentDir), stem, ext);
        std::ofstream out(target.string(), std::ios::binary | std::ios::trunc);
        if (!out) return;
        if (!initial.empty()) out << initial;
        out.close();
        if (ctx.noteChange) ctx.noteChange("Asset created: " + target.filename().string());
        Refresh();
        if (ctx.openAssetTab) ctx.openAssetTab(target.string());
    }

    void createEquinoxAssetInCurrentDir(EquinoxAssetType type)
    {
        const std::string stem = EquinoxFileManager::DefaultStem(type);
        const std::string ext  = EquinoxFileManager::DefaultExtension(type);
        if (ext.empty()) return;
        createAssetInCurrentDir(stem, ext, EquinoxFileManager::BuildDefaultDocument(type, stem));
    }

    void openEquinoxWorkspace()
    {
        if (!ctx.pm.isOpen) return;
        std::error_code ec;
        fs::path dir = fs::path(ctx.pm.AssetsDir()) / "equinox";
        fs::create_directories(dir, ec);
        openDirectory(dir);
        for (const auto& entry : fs::directory_iterator(dir, ec)) {
            if (ec) break;
            if (!entry.is_regular_file()) continue;
            if (!EquinoxFileManager::IsEquinoxAssetPath(entry.path().string())) continue;
            if (ctx.openAssetTab) ctx.openAssetTab(entry.path().string());
            return;
        }
        createEquinoxAssetInCurrentDir(EquinoxAssetType::ShaderComposer);
    }

    void SaveCache(IniFile& ini) const
    {
        ini.Set("ContentBrowser", "CurrentDir", cbCurrentDir);
        ini.Set("ContentBrowser", "SelectedPath", cbSelectedPath);
        ini.Set("ContentBrowser", "TypeFilter", std::to_string(cbTypeFilter));
        ini.Set("ContentBrowser", "ViewMode", std::to_string(cbViewMode));
    }

    void LoadCache(IniFile& ini)
    {
        cbCurrentDir   = ini.Get("ContentBrowser", "CurrentDir", cbCurrentDir);
        cbSelectedPath = ini.Get("ContentBrowser", "SelectedPath", cbSelectedPath);
        cbTypeFilter   = ini.GetInt("ContentBrowser", "TypeFilter", cbTypeFilter);
        cbViewMode     = ini.GetInt("ContentBrowser", "ViewMode", cbViewMode);
    }

private:
    EditorContext& ctx;

    bool ensureDirValid()
    {
        if (!ctx.pm.isOpen) return false;
        fs::path root = ctx.contentRootDir();
        if (root.empty()) return false;
        std::error_code ec;
        if (cbCurrentDir.empty()) cbCurrentDir = root.string();
        fs::path cur(cbCurrentDir);
        if (!fs::exists(cur, ec) || !fs::is_directory(cur, ec)) { cbCurrentDir = root.string(); return true; }
        fs::path rel = fs::relative(cur, root, ec);
        if (ec || rel.empty() || rel.string().rfind("..", 0) == 0) { cbCurrentDir = root.string(); return true; }
        return true;
    }

    void goParent()
    {
        if (!ctx.pm.isOpen || !ensureDirValid()) return;
        fs::path root = ctx.contentRootDir();
        fs::path cur(cbCurrentDir);
        if (cur == root) return;
        fs::path parent = cur.parent_path();
        if (parent.empty()) parent = root;
        openDirectory(parent);
    }

    std::string treePathForNode(TreeNode* node) const
    {
        if (!node) return {};
        for (const auto& it : cbTreeNodePaths)
            if (it.first == node) return it.second;
        return {};
    }

    void buildTreeRecursive(TreeNode* parent, const fs::path& dir)
    {
        std::vector<fs::directory_entry> dirs;
        std::error_code ec;
        for (auto it = fs::directory_iterator(dir, ec); !ec && it != fs::directory_iterator(); ++it)
            if (it->is_directory()) dirs.push_back(*it);

        auto byName = [](const fs::directory_entry& a, const fs::directory_entry& b) {
            return LightningEditor::ToLowerCopy(a.path().filename().string()) <
                   LightningEditor::ToLowerCopy(b.path().filename().string());
        };
        std::sort(dirs.begin(), dirs.end(), byName);

        for (const auto& d : dirs) {
            TreeNode* tn = parent->AddChild(d.path().filename().string());
            tn->expanded = false;
            cbTreeNodePaths.push_back({ tn, d.path().string() });
            buildTreeRecursive(tn, d.path());
        }
    }

    void buildGridView(const std::vector<fs::directory_entry>& dirs,
                       const std::vector<fs::directory_entry>& files,
                       float rightW)
    {
        float gap = 8.f;
        int cols = std::max(1, (int)((rightW - gap) / (110.f + gap)));
        float cellW = (rightW - gap * (cols + 1)) / cols;

        pCBGrid = pCBScroll->Add<Grid>(0.f, 0.f, rightW - 8.f, cols, 84.f, gap, gap, gap, gap);
        pCBGrid->stretchCells = true;
        pCBGrid->autoH = true;

        for (const auto& d : dirs) {
            std::string label = d.path().filename().string();
            auto* btn = pCBGrid->Add<Button>(0.f, 0.f, cellW, 84.f, label.c_str());
            btn->icon = resolveDirIcon(d.path());
            btn->iconTop = true; btn->iconSize = 28.f;
            btn->SetColor(125, 170, 255);
            if (cbSelectedPath == d.path().string()) btn->SetColor(255, 220, 120);
            btn->onClick = [this, d]{ handleEntryClick(d.path(), true); };
        }

        for (const auto& f : files) {
            if (!matchesTypeFilter(f.path())) continue;
            std::string name = f.path().filename().string();
            std::string label = name.size() > 16 ? name.substr(0, 13) + "..." : name;
            auto* btn = pCBGrid->Add<Button>(0.f, 0.f, cellW, 84.f, label.c_str());
            btn->icon = resolveFileIcon(f.path());
            btn->iconTop = true; btn->iconSize = 28.f;
            std::string ext = f.path().extension().string();
            std::string lExt = LightningEditor::ToLowerCopy(ext);
            if (lExt == ".lescene") btn->SetColor(75, 195, 75);
            else { auto tab = LightningEditor::BuildAssetTab(f.path().string()); btn->SetColor(tab.accent.r, tab.accent.g, tab.accent.b); }
            if (cbSelectedPath == f.path().string()) btn->SetColor(255, 220, 120);
            btn->onClick = [this, f, ext]{
                handleEntryClick(f.path(), false);
                cbDragFile = f.path().string(); cbDragExt = ext; cbDragging = true;
            };
        }
    }

    void buildListView(const std::vector<fs::directory_entry>& dirs,
                       const std::vector<fs::directory_entry>& files,
                       float rightW)
    {
        float rowH = gStyle.lineH + 2.f;
        for (const auto& d : dirs) {
            auto* btn = pCBScroll->Add<Button>(0.f, 0.f, rightW, rowH, d.path().filename().string().c_str());
            btn->icon = resolveDirIcon(d.path());
            btn->iconTop = false; btn->iconSize = 14.f;
            btn->SetColor(125, 170, 255);
            if (cbSelectedPath == d.path().string()) btn->SetColor(255, 220, 120);
            btn->onClick = [this, d]{ handleEntryClick(d.path(), true); };
        }
        for (const auto& f : files) {
            if (!matchesTypeFilter(f.path())) continue;
            std::string ext = f.path().extension().string();
            auto* btn = pCBScroll->Add<Button>(0.f, 0.f, rightW, rowH, f.path().filename().string().c_str());
            btn->icon = resolveFileIcon(f.path());
            btn->iconTop = false; btn->iconSize = 14.f;
            auto tab = LightningEditor::BuildAssetTab(f.path().string());
            btn->SetColor(tab.accent.r, tab.accent.g, tab.accent.b);
            if (cbSelectedPath == f.path().string()) btn->SetColor(255, 220, 120);
            btn->onClick = [this, f, ext]{
                handleEntryClick(f.path(), false);
                cbDragFile = f.path().string(); cbDragExt = ext; cbDragging = true;
            };
        }
    }
};
