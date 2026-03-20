#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace LightningEditor {

namespace fs = std::filesystem;

enum class EditorTabKind {
    Scene,
    Script,
    Texture,
    Shader,
    Material,
    Particle,
    Prefab,
    StaticMesh,
    SkeletalMesh,
    Config,
    Generic,
};

enum class EditorTabLayout {
    SceneWorkspace,
    DocumentWorkspace,
};

struct EditorTabAccent {
    std::uint8_t r = 255;
    std::uint8_t g = 255;
    std::uint8_t b = 255;
};

struct EditorTabDescriptor {
    std::string id;
    std::string label;
    std::string path;
    std::string extension;
    EditorTabKind kind = EditorTabKind::Generic;
    EditorTabLayout layout = EditorTabLayout::DocumentWorkspace;
    EditorTabAccent accent;
    bool closable = true;
    bool active = false;
    bool dirty = false;            // true when in-memory content differs from disk
    std::vector<std::string> pendingLines; // buffered edit, flushed on save

    bool IsSceneWorkspace() const { return layout == EditorTabLayout::SceneWorkspace; }
    bool IsDocumentWorkspace() const { return layout == EditorTabLayout::DocumentWorkspace; }

    // Display label including dirty indicator
    std::string DisplayLabel() const { return dirty ? label + " *" : label; }
};

inline std::string ToLowerCopy(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}

inline std::string SceneTabLabel(const std::string& sceneRelPath)
{
    if (sceneRelPath.empty()) return "Cena";
    std::string label = fs::path(sceneRelPath).filename().string();
    return label.empty() ? "Cena" : label;
}

inline EditorTabDescriptor BuildSceneTab(const std::string& sceneRelPath)
{
    EditorTabDescriptor tab;
    tab.id = "scene:primary";
    tab.label = SceneTabLabel(sceneRelPath);
    tab.path = sceneRelPath;
    tab.extension = ".lescene";
    tab.kind = EditorTabKind::Scene;
    tab.layout = EditorTabLayout::SceneWorkspace;
    tab.accent = { 255, 255, 255 };
    tab.closable = false;
    tab.active = true;
    return tab;
}

inline EditorTabDescriptor BuildAssetTab(const std::string& absPath)
{
    EditorTabDescriptor tab;
    tab.id = absPath;
    tab.path = absPath;
    tab.label = fs::path(absPath).filename().string();
    tab.extension = ToLowerCopy(fs::path(absPath).extension().string());
    tab.layout = EditorTabLayout::DocumentWorkspace;
    tab.closable = true;

    if (tab.extension == ".spark" || tab.extension == ".cs") {
        tab.kind = EditorTabKind::Script;
        tab.accent = { 255, 138, 28 };
    } else if (tab.extension == ".png" || tab.extension == ".bmp" ||
               tab.extension == ".jpg" || tab.extension == ".jpeg" ||
               tab.extension == ".tga") {
        tab.kind = EditorTabKind::Texture;
        tab.accent = { 210, 65, 65 };
    } else if (tab.extension == ".vert" || tab.extension == ".frag" ||
               tab.extension == ".spv") {
        tab.kind = EditorTabKind::Shader;
        tab.accent = { 72, 190, 90 };
    } else if (tab.extension == ".mat" || tab.extension == ".material") {
        tab.kind = EditorTabKind::Material;
        tab.accent = { 60, 150, 80 };
    } else if (tab.extension == ".hurricane" || tab.extension == ".particle") {
        tab.kind = EditorTabKind::Particle;
        tab.accent = { 145, 80, 210 };
    } else if (tab.extension == ".prefab" || tab.extension == ".lprefab") {
        tab.kind = EditorTabKind::Prefab;
        tab.accent = { 220, 190, 70 };
    } else if (tab.extension == ".obj" || tab.extension == ".fbx" ||
               tab.extension == ".gltf" || tab.extension == ".glb") {
        tab.kind = EditorTabKind::StaticMesh;
        tab.accent = { 110, 180, 245 };
    } else if (tab.extension == ".anim" || tab.extension == ".skel") {
        tab.kind = EditorTabKind::SkeletalMesh;
        tab.accent = { 220, 120, 185 };
    } else if (tab.extension == ".ini" || tab.extension == ".json" ||
               tab.extension == ".toml" || tab.extension == ".yaml" ||
               tab.extension == ".yml") {
        tab.kind = EditorTabKind::Config;
        tab.accent = { 140, 140, 150 };
    } else {
        tab.kind = EditorTabKind::Generic;
        tab.accent = { 185, 185, 195 };
    }

    return tab;
}

inline bool IsTextDocument(const EditorTabDescriptor& tab)
{
    if (tab.kind == EditorTabKind::Script ||
        tab.kind == EditorTabKind::Material ||
        tab.kind == EditorTabKind::Config) {
        return true;
    }

    if (tab.kind == EditorTabKind::Shader) {
        return tab.extension != ".spv";
    }

    return tab.extension == ".txt" || tab.extension == ".md" ||
           tab.extension == ".log" || tab.extension == ".csv";
}

class EditorTabManager {
public:
    EditorTabManager()
    {
        ResetToSceneOnly("");
    }

    void ResetToSceneOnly(const std::string& sceneRelPath)
    {
        tabs_.clear();
        tabs_.push_back(BuildSceneTab(sceneRelPath));
        activeIndex_ = 0;
    }

    void SyncPrimarySceneTab(const std::string& sceneRelPath)
    {
        if (tabs_.empty()) {
            ResetToSceneOnly(sceneRelPath);
            return;
        }

        EditorTabDescriptor sceneTab = BuildSceneTab(sceneRelPath);
        sceneTab.active = (activeIndex_ == 0);
        tabs_[0] = sceneTab;
        syncActiveFlags();
    }

    int Activate(int idx)
    {
        if (idx < 0 || idx >= static_cast<int>(tabs_.size())) return activeIndex_;
        activeIndex_ = idx;
        syncActiveFlags();
        return activeIndex_;
    }

    int OpenAsset(const std::string& absPath)
    {
        const std::string lowerExt = ToLowerCopy(fs::path(absPath).extension().string());
        if (lowerExt == ".lescene") {
            return Activate(0);
        }

        int idx = FindById(absPath);
        if (idx < 0) {
            tabs_.push_back(BuildAssetTab(absPath));
            idx = static_cast<int>(tabs_.size()) - 1;
        }
        return Activate(idx);
    }

    void Close(int idx)
    {
        if (idx <= 0 || idx >= static_cast<int>(tabs_.size())) return;
        if (!tabs_[idx].closable) return;

        tabs_.erase(tabs_.begin() + idx);
        if (activeIndex_ >= static_cast<int>(tabs_.size())) {
            activeIndex_ = static_cast<int>(tabs_.size()) - 1;
        }
        if (activeIndex_ < 0) activeIndex_ = 0;
        syncActiveFlags();
    }

    int ActiveIndex() const { return activeIndex_; }

    bool HasActiveDocument() const
    {
        const EditorTabDescriptor* tab = ActiveTab();
        return tab && tab->IsDocumentWorkspace();
    }

    const EditorTabDescriptor* ActiveTab() const
    {
        if (tabs_.empty()) return nullptr;
        if (activeIndex_ < 0 || activeIndex_ >= static_cast<int>(tabs_.size())) return nullptr;
        return &tabs_[activeIndex_];
    }

    const std::vector<EditorTabDescriptor>& Tabs() const { return tabs_; }

    // Mark active document tab as dirty and buffer the edited lines.
    void MarkDirty(const std::vector<std::string>& lines)
    {
        if (activeIndex_ < 0 || activeIndex_ >= static_cast<int>(tabs_.size())) return;
        EditorTabDescriptor& tab = tabs_[activeIndex_];
        if (!tab.IsDocumentWorkspace()) return;
        tab.dirty = true;
        tab.pendingLines = lines;
    }

    // Flush one dirty tab to disk and mark it clean. Returns false if write failed.
    bool SaveTab(int idx)
    {
        if (idx < 0 || idx >= static_cast<int>(tabs_.size())) return false;
        EditorTabDescriptor& tab = tabs_[idx];
        if (!tab.dirty || tab.path.empty()) return false;
        if (WriteTextFile(tab.path, tab.pendingLines)) {
            tab.dirty = false;
            tab.pendingLines.clear();
            return true;
        }
        return false;
    }

    // Flush all dirty document tabs. Returns number of files written.
    int SaveAllDirty()
    {
        int saved = 0;
        for (int i = 0; i < static_cast<int>(tabs_.size()); ++i) {
            if (tabs_[i].dirty) saved += SaveTab(i) ? 1 : 0;
        }
        return saved;
    }

    bool HasDirtyTabs() const
    {
        for (const auto& tab : tabs_)
            if (tab.dirty) return true;
        return false;
    }

private:
    int FindById(const std::string& id) const
    {
        for (int i = 0; i < static_cast<int>(tabs_.size()); ++i) {
            if (tabs_[i].id == id) return i;
        }
        return -1;
    }

    void syncActiveFlags()
    {
        for (int i = 0; i < static_cast<int>(tabs_.size()); ++i) {
            tabs_[i].active = (i == activeIndex_);
        }
    }

    static bool WriteTextFile(const std::string& path, const std::vector<std::string>& lines)
    {
        std::ofstream out(path, std::ios::binary | std::ios::trunc);
        if (!out) return false;
        for (std::size_t i = 0; i < lines.size(); ++i) {
            if (i) out << '\n';
            out << lines[i];
        }
        return static_cast<bool>(out);
    }

    std::vector<EditorTabDescriptor> tabs_;
    int activeIndex_ = 0;
};

} // namespace LightningEditor