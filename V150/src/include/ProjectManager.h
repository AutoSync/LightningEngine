// ProjectManager.h — Create, open, save and close Lightning Engine projects.
// Manages the project folder structure and scene serialization (.lescene).
//
// Scene format (.lescene) — simple indented text:
//   NODE <type> "<name>"
//     TRANSFORM px py pz  rx ry rz  sx sy sz
//     TAG "<tag>"
//     ACTIVE <0|1>
//     SCRIPT "<relative/path/to/script.cs>"
//     NODE ...
//     END
//   END
#pragma once
#include "ProjectConfig.h"
#include "Level.h"
#include <string>
#include <vector>

namespace LightningEngine {

// Template applied when creating a new project.
enum class ProjectTemplate { Empty, Game2D, Game3D };

class ProjectManager {
public:
    ProjectConfig project;
    GameConfig    game;
    EditorConfig  editor;
    bool          isOpen = false;

    // ── Project lifetime ────────────────────────────────────────────────────

    // Create a new project at `folderPath` (will be created if absent).
    // Initialises folder structure, writes INI files, and seeds template content.
    bool Create(const std::string& folderPath, const std::string& name,
                ProjectTemplate tpl = ProjectTemplate::Empty);

    // Open an existing project.  Pass either the project folder or project.ini.
    bool Open(const std::string& path);

    // Save all three INI files back to disk.
    bool Save();

    // Close and reset state.
    void Close();

    // ── Scene serialization ─────────────────────────────────────────────────

    // Serialise every root node in `level` to a .lescene file.
    // `scenePath` is relative to the project root (e.g. "scenes/main.lescene").
    bool SaveScene(const std::string& scenePath, const Level& level);

    // Deserialise a .lescene file, adding nodes to `level`.
    // Clears existing level nodes first.
    bool LoadScene(const std::string& scenePath, Level& level);

    // ── Prefab serialization ─────────────────────────────────────────────────
    // Prefabs are single-node subtrees saved as .lprefab files.
    // Format reuses the .lescene grammar with a PREFAB header.

    // Save a node subtree to `prefabPath` (absolute or relative to project root).
    bool SavePrefab(const std::string& prefabPath, const Node& root);

    // Load a prefab and return the root node (nullptr on failure).
    std::unique_ptr<Node> LoadPrefab(const std::string& prefabPath);

    // ── Path helpers ────────────────────────────────────────────────────────
    std::string ProjectIni()  const { return project.rootPath + "/project.ini"; }
    std::string GameIni()     const { return project.rootPath + "/game.ini";    }
    std::string EditorIni()   const { return project.rootPath + "/editor.ini";  }
    std::string ScenesDir()   const { return project.rootPath + "/scenes";      }
    std::string AssetsDir()   const { return project.rootPath + "/assets";      }
    std::string ScriptsDir()  const { return project.rootPath + "/scripts";     }
    std::string PrefabsDir()  const { return project.rootPath + "/assets/prefabs"; }
    std::string AbsScene(const std::string& rel) const
    { return project.rootPath + "/" + rel; }

    // ── Global recent-projects list ─────────────────────────────────────────
    // Stored in %APPDATA%/LightningEngine/recent.ini (Windows) or ~/.config
    static std::vector<std::string> GetRecentProjects();
    static void AddRecentProject(const std::string& folderPath);

private:
    ProjectTemplate pendingTemplate = ProjectTemplate::Empty;

    void createFolderStructure();
    void writeDefaultFiles();
    std::string resolveProjectIni(const std::string& path) const;

    // Scene I/O helpers — recursive for children
    void writeNode (std::ostream& out, const Node& node, int depth) const;
    void readNodes (std::istream& in,  Level& level) const;
};

} // namespace LightningEngine
