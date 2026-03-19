#pragma warning(disable: 4996)  // getenv: MSVC unsafe function warning
#include "../include/ProjectManager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

namespace LightningEngine {

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

static std::string recentProjectsPath()
{
    // Windows: %APPDATA%\LightningEngine\recent.ini
    const char* appdata = std::getenv("APPDATA");
    std::string base = appdata ? std::string(appdata) : ".";
    return base + "/LightningEngine/recent.ini";
}

static std::string indent(int depth)
{
    return std::string(depth * 2, ' ');
}

// ─────────────────────────────────────────────────────────────────────────────
// Create / Open / Save / Close
// ─────────────────────────────────────────────────────────────────────────────

bool ProjectManager::Create(const std::string& folderPath, const std::string& name,
                            ProjectTemplate tpl)
{
    project.rootPath  = fs::absolute(folderPath).string();
    project.name      = name;
    project.version   = "0.1.0";
    project.lastScene = "scenes/main.lescene";
    game.title        = name;
    editor.lastPath   = "";
    pendingTemplate   = tpl;

    std::error_code ec;
    fs::create_directories(folderPath, ec);
    if (ec) {
        std::cerr << "[ProjectManager] Cannot create folder: " << ec.message() << "\n";
        return false;
    }

    createFolderStructure();
    writeDefaultFiles();
    isOpen = true;
    AddRecentProject(project.rootPath);
    return true;
}

bool ProjectManager::Open(const std::string& path)
{
    std::string ini = resolveProjectIni(path);
    if (ini.empty()) {
        std::cerr << "[ProjectManager] project.ini not found at: " << path << "\n";
        return false;
    }
    project.rootPath = fs::path(ini).parent_path().string();
    if (!project.Load(ini)) {
        std::cerr << "[ProjectManager] Failed to parse project.ini\n";
        return false;
    }
    game.Load(GameIni());
    editor.Load(EditorIni());
    isOpen = true;
    AddRecentProject(project.rootPath);
    return true;
}

bool ProjectManager::Save()
{
    if (!isOpen) return false;
    bool ok = true;
    ok &= project.Save(ProjectIni());
    ok &= game.Save(GameIni());
    ok &= editor.Save(EditorIni());
    return ok;
}

void ProjectManager::Close()
{
    isOpen = false;
    project = ProjectConfig{};
    game    = GameConfig{};
    editor  = EditorConfig{};
}

// ─────────────────────────────────────────────────────────────────────────────
// Scene serialisation
// ─────────────────────────────────────────────────────────────────────────────

void ProjectManager::writeNode(std::ostream& out, const Node& node, int depth) const
{
    std::string ind = indent(depth);
    out << ind << "NODE Node \"" << node.name << "\"\n";
    const auto& t = node.transform;
    out << ind << "  TRANSFORM "
        << t.Position.x << " " << t.Position.y << " " << t.Position.z << "  "
        << t.Rotation.x << " " << t.Rotation.y << " " << t.Rotation.z << "  "
        << t.Scale.x    << " " << t.Scale.y    << " " << t.Scale.z    << "\n";
    out << ind << "  TAG \"" << node.tag << "\"\n";
    out << ind << "  ACTIVE " << (node.active ? 1 : 0) << "\n";

    for (const auto* child : node.GetChildren())
        writeNode(out, *child, depth + 1);

    out << ind << "END\n";
}

bool ProjectManager::SaveScene(const std::string& scenePath, const Level& level)
{
    if (!isOpen) return false;
    std::string absPath = AbsScene(scenePath);
    fs::create_directories(fs::path(absPath).parent_path());

    std::ofstream out(absPath);
    if (!out) {
        std::cerr << "[ProjectManager] Cannot write scene: " << absPath << "\n";
        return false;
    }
    out << "# Lightning Engine Scene 1.0\n";
    for (const auto& n : level.GetNodes())
        writeNode(out, *n, 0);

    project.lastScene = scenePath;
    return true;
}

// Simple tokeniser for .lescene lines
static std::vector<std::string> tokeniseLine(const std::string& line)
{
    std::vector<std::string> toks;
    std::istringstream ss(line);
    std::string tok;
    while (ss >> std::quoted(tok))
        toks.push_back(tok);
    return toks;
}

bool ProjectManager::LoadScene(const std::string& scenePath, Level& level)
{
    if (!isOpen) return false;
    std::string absPath = AbsScene(scenePath);
    std::ifstream in(absPath);
    if (!in) {
        std::cerr << "[ProjectManager] Cannot open scene: " << absPath << "\n";
        return false;
    }

    // Simple stack-based recursive reader
    struct StackEntry { std::unique_ptr<Node> node; Node* parent; };
    std::vector<StackEntry> stack;
    std::vector<std::unique_ptr<Node>> roots;

    auto finishNode = [&]() {
        if (stack.empty()) return;
        auto entry = std::move(stack.back());
        stack.pop_back();
        if (entry.parent) {
            entry.parent->AddChild(std::move(entry.node));
        } else {
            roots.push_back(std::move(entry.node));
        }
    };

    std::string line;
    while (std::getline(in, line)) {
        // strip leading whitespace + comments
        size_t start = line.find_first_not_of(" \t");
        if (start == std::string::npos) continue;
        line = line.substr(start);
        if (line[0] == '#') continue;

        auto toks = tokeniseLine(line);
        if (toks.empty()) continue;

        if (toks[0] == "NODE" && toks.size() >= 3) {
            // toks: NODE <type> "<name>"
            auto n = std::make_unique<Node>(toks[2]);
            Node* parent = stack.empty() ? nullptr : stack.back().node.get();
            stack.push_back({ std::move(n), parent });

        } else if (toks[0] == "TRANSFORM" && toks.size() >= 10 && !stack.empty()) {
            auto& t = stack.back().node->transform;
            t.Position = Lightning::V3(std::stof(toks[1]), std::stof(toks[2]), std::stof(toks[3]));
            t.Rotation = Lightning::V3(std::stof(toks[4]), std::stof(toks[5]), std::stof(toks[6]));
            t.Scale    = Lightning::V3(std::stof(toks[7]), std::stof(toks[8]), std::stof(toks[9]));

        } else if (toks[0] == "TAG" && toks.size() >= 2 && !stack.empty()) {
            stack.back().node->tag = toks[1];

        } else if (toks[0] == "ACTIVE" && toks.size() >= 2 && !stack.empty()) {
            stack.back().node->active = toks[1] != "0";

        } else if (toks[0] == "END") {
            finishNode();
        }
    }
    // Flush any unclosed nodes
    while (!stack.empty()) finishNode();

    for (auto& n : roots)
        level.AddNode(std::move(n));

    project.lastScene = scenePath;
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Prefab serialization
// ─────────────────────────────────────────────────────────────────────────────

bool ProjectManager::SavePrefab(const std::string& prefabPath, const Node& root)
{
    // Resolve absolute path: if not absolute, treat as relative to project root.
    std::string absPath = fs::path(prefabPath).is_absolute()
                        ? prefabPath
                        : (project.rootPath + "/" + prefabPath);
    fs::create_directories(fs::path(absPath).parent_path());

    std::ofstream out(absPath);
    if (!out) {
        std::cerr << "[ProjectManager] Cannot write prefab: " << absPath << "\n";
        return false;
    }
    out << "# Lightning Engine Prefab 1.0\n";
    out << "PREFAB \"" << root.name << "\"\n";
    writeNode(out, root, 0);
    return true;
}

std::unique_ptr<Node> ProjectManager::LoadPrefab(const std::string& prefabPath)
{
    std::string absPath = fs::path(prefabPath).is_absolute()
                        ? prefabPath
                        : (project.rootPath + "/" + prefabPath);
    std::ifstream in(absPath);
    if (!in) {
        std::cerr << "[ProjectManager] Cannot open prefab: " << absPath << "\n";
        return nullptr;
    }

    struct StackEntry { std::unique_ptr<Node> node; Node* parent; };
    std::vector<StackEntry> stack;
    std::unique_ptr<Node>   result;

    auto finishNode = [&]() {
        if (stack.empty()) return;
        auto entry = std::move(stack.back());
        stack.pop_back();
        if (entry.parent) {
            entry.parent->AddChild(std::move(entry.node));
        } else {
            result = std::move(entry.node);
        }
    };

    std::string line;
    while (std::getline(in, line)) {
        size_t start = line.find_first_not_of(" \t");
        if (start == std::string::npos) continue;
        line = line.substr(start);
        if (line[0] == '#') continue;

        auto toks = tokeniseLine(line);
        if (toks.empty()) continue;

        if (toks[0] == "PREFAB") {
            // Header line — skip, name taken from first NODE.
        } else if (toks[0] == "NODE" && toks.size() >= 3) {
            auto n = std::make_unique<Node>(toks[2]);
            Node* parent = stack.empty() ? nullptr : stack.back().node.get();
            stack.push_back({ std::move(n), parent });
        } else if (toks[0] == "TRANSFORM" && toks.size() >= 10 && !stack.empty()) {
            auto& t = stack.back().node->transform;
            t.Position = Lightning::V3(std::stof(toks[1]), std::stof(toks[2]), std::stof(toks[3]));
            t.Rotation = Lightning::V3(std::stof(toks[4]), std::stof(toks[5]), std::stof(toks[6]));
            t.Scale    = Lightning::V3(std::stof(toks[7]), std::stof(toks[8]), std::stof(toks[9]));
        } else if (toks[0] == "TAG" && toks.size() >= 2 && !stack.empty()) {
            stack.back().node->tag = toks[1];
        } else if (toks[0] == "ACTIVE" && toks.size() >= 2 && !stack.empty()) {
            stack.back().node->active = toks[1] != "0";
        } else if (toks[0] == "END") {
            finishNode();
        }
    }
    while (!stack.empty()) finishNode();
    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// Private helpers
// ─────────────────────────────────────────────────────────────────────────────

void ProjectManager::createFolderStructure()
{
    for (const auto& sub : { "/scenes", "/assets", "/scripts",
                              "/assets/textures", "/assets/audio",
                              "/assets/models",   "/assets/shaders" }) {
        std::error_code ec;
        fs::create_directories(project.rootPath + sub, ec);
    }
}

void ProjectManager::writeDefaultFiles()
{
    project.Save(ProjectIni());
    game.Save(GameIni());
    editor.Save(EditorIni());

    // Default .gitignore
    std::ofstream gi(project.rootPath + "/.gitignore");
    if (gi) gi << "assets/shaders/*.spv\n";

    // Scene and scripts seeded from template
    std::string sceneAbs = project.rootPath + "/" + project.lastScene;
    fs::create_directories(fs::path(sceneAbs).parent_path());

    if (pendingTemplate == ProjectTemplate::Game2D) {
        // ── 2D template: one sprite node with a Nucleo script ───────────────
        std::ofstream sc(sceneAbs);
        if (sc) {
            sc << "# Lightning Engine Scene 1.0\n";
            sc << "NODE Node2D \"Player\"\n";
            sc << "  TRANSFORM 640 360 0  0 0 0  1 1 1\n";
            sc << "  TAG \"player\"\n";
            sc << "  ACTIVE 1\n";
            sc << "  SCRIPT \"scripts/PlayerController.spark\"\n";
            sc << "END\n";
        }
        // Starter script
        std::string scriptPath = project.rootPath + "/scripts/PlayerController.spark";
        std::ofstream ss(scriptPath);
        if (ss) {
            ss << "class PlayerController {\n";
            ss << "    var speed = 200\n";
            ss << "\n";
            ss << "    void OnStart() {\n";
            ss << "        Log(\"PlayerController started\")\n";
            ss << "    }\n";
            ss << "\n";
            ss << "    void Update(float dt) {\n";
            ss << "        var dx = 0\n";
            ss << "        var dy = 0\n";
            ss << "        if (Input.IsKeyDown(80)) { dx = -1 }\n";  // SDL_SCANCODE_LEFT = 80
            ss << "        if (Input.IsKeyDown(79)) { dx =  1 }\n";  // SDL_SCANCODE_RIGHT = 79
            ss << "        if (Input.IsKeyDown(82)) { dy = -1 }\n";  // SDL_SCANCODE_UP = 82
            ss << "        if (Input.IsKeyDown(81)) { dy =  1 }\n";  // SDL_SCANCODE_DOWN = 81
            ss << "        Node.Move(dx * speed * dt, dy * speed * dt, 0)\n";
            ss << "    }\n";
            ss << "\n";
            ss << "    void OnDestroy() {}\n";
            ss << "}\n";
        }
    } else if (pendingTemplate == ProjectTemplate::Game3D) {
        // ── 3D template: camera node + cube node ────────────────────────────
        std::ofstream sc(sceneAbs);
        if (sc) {
            sc << "# Lightning Engine Scene 1.0\n";
            sc << "NODE Node3D \"Camera\"\n";
            sc << "  TRANSFORM 0 2 -5  0 0 0  1 1 1\n";
            sc << "  TAG \"camera\"\n";
            sc << "  ACTIVE 1\n";
            sc << "END\n";
            sc << "NODE Node3D \"Cube\"\n";
            sc << "  TRANSFORM 0 0 0  0 0 0  1 1 1\n";
            sc << "  TAG \"\"\n";
            sc << "  ACTIVE 1\n";
            sc << "END\n";
        }
        // Starter 3D camera controller script
        std::string scriptPath = project.rootPath + "/scripts/FreeCam.spark";
        std::ofstream ss(scriptPath);
        if (ss) {
            ss << "class FreeCam {\n";
            ss << "    var speed = 5\n";
            ss << "\n";
            ss << "    void OnStart() {\n";
            ss << "        Log(\"FreeCam started\")\n";
            ss << "    }\n";
            ss << "\n";
            ss << "    void Update(float dt) {\n";
            ss << "        var fwd = 0\n";
            ss << "        var side = 0\n";
            ss << "        if (Input.IsKeyDown(26)) { fwd  =  1 }\n";  // W
            ss << "        if (Input.IsKeyDown(22)) { fwd  = -1 }\n";  // S
            ss << "        if (Input.IsKeyDown(4))  { side = -1 }\n";  // A
            ss << "        if (Input.IsKeyDown(7))  { side =  1 }\n";  // D
            ss << "        Node.Move(side * speed * dt, 0, fwd * speed * dt)\n";
            ss << "    }\n";
            ss << "\n";
            ss << "    void OnDestroy() {}\n";
            ss << "}\n";
        }
    } else {
        // ── Empty template ───────────────────────────────────────────────────
        std::ofstream sc(sceneAbs);
        if (sc) sc << "# Lightning Engine Scene 1.0\n";
    }
}

std::string ProjectManager::resolveProjectIni(const std::string& path) const
{
    fs::path p(path);
    if (fs::is_regular_file(p)) {
        if (p.filename() == "project.ini") return fs::absolute(p).string();
    }
    // Try path/project.ini
    fs::path candidate = p / "project.ini";
    if (fs::exists(candidate)) return fs::absolute(candidate).string();
    return "";
}

// ─────────────────────────────────────────────────────────────────────────────
// Recent projects
// ─────────────────────────────────────────────────────────────────────────────

std::vector<std::string> ProjectManager::GetRecentProjects()
{
    IniFile f;
    if (!f.Load(recentProjectsPath())) return {};
    std::vector<std::string> out;
    for (int i = 0; i < 20; ++i) {
        std::string v = f.Get("Recent", "p" + std::to_string(i), "");
        if (!v.empty()) out.push_back(v);
    }
    return out;
}

void ProjectManager::AddRecentProject(const std::string& folderPath)
{
    std::string rPath = recentProjectsPath();
    fs::create_directories(fs::path(rPath).parent_path());

    IniFile f;
    f.Load(rPath);

    // Collect existing, deduplicate, prepend new
    std::vector<std::string> list = { folderPath };
    for (int i = 0; i < 20; ++i) {
        std::string v = f.Get("Recent", "p" + std::to_string(i), "");
        if (!v.empty() && v != folderPath) list.push_back(v);
    }
    if (list.size() > 20) list.resize(20);

    IniFile out;
    for (int i = 0; i < (int)list.size(); ++i)
        out.Set("Recent", "p" + std::to_string(i), list[i]);
    out.Save(rPath);
}

} // namespace LightningEngine
