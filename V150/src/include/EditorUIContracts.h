// EditorUIContracts.h — Contracts for editor UI extensibility (commands, layout).
#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <vector>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <iomanip>

namespace LightningEngine {

// Command scope for editor commands
enum class CommandScope {
    Global,
    Project,
    Scene
};

// Editor command definition
struct EditorCommand {
    std::string id;
    std::string name;
    std::string shortcut;  // e.g., "Ctrl+S"
    CommandScope scope = CommandScope::Global;
    std::function<void()> action;
};

// Central registry for editor commands and shortcuts
class EditorCommandRegistry {
public:
    bool RegisterCommand(const EditorCommand& cmd) {
        if (commands.find(cmd.id) != commands.end()) return false;
        commands[cmd.id] = cmd;
        return true;
    }

    bool ExecuteCommand(const std::string& id) {
        auto it = commands.find(id);
        if (it == commands.end()) return false;
        if (it->second.action) {
            it->second.action();
            return true;
        }
        return false;
    }

    const EditorCommand* GetCommand(const std::string& id) const {
        auto it = commands.find(id);
        return it != commands.end() ? &it->second : nullptr;
    }

    std::vector<EditorCommand> GetAllCommands() const {
        std::vector<EditorCommand> result;
        for (const auto& pair : commands) {
            result.push_back(pair.second);
        }
        return result;
    }

private:
    std::unordered_map<std::string, EditorCommand> commands;
};

// Layout data structure for persistence
struct EditorLayoutData {
    struct DockNodeData {
        std::string id;
        float x, y, w, h;
        bool visible = true;
    };

    struct TabData {
        std::string id;
        bool active = false;
    };

    std::vector<DockNodeData> dockNodes;
    std::vector<TabData> tabs;
    std::unordered_map<std::string, std::string> preferences;  // key-value pairs
};

// Persistence store for editor layout per project
class EditorLayoutStore {
public:
    EditorLayoutStore(const std::string& projectRoot) : projectRoot(projectRoot) {}

    bool SaveLayout(const EditorLayoutData& data) {
        try {
            std::ofstream fout(layoutFilePath(), std::ios::trunc);
            if (!fout.is_open()) return false;

            // Simple line-based format to avoid external dependencies.
            for (const auto& node : data.dockNodes) {
                fout << "DOCK " << std::quoted(node.id) << ' '
                     << node.x << ' ' << node.y << ' ' << node.w << ' ' << node.h << ' ' << (node.visible ? 1 : 0) << '\n';
            }

            for (const auto& tab : data.tabs) {
                fout << "TAB " << std::quoted(tab.id) << ' ' << (tab.active ? 1 : 0) << '\n';
            }

            for (const auto& pref : data.preferences) {
                fout << "PREF " << std::quoted(pref.first) << ' ' << std::quoted(pref.second) << '\n';
            }

            return fout.good();
        } catch (...) {
            return false;
        }
    }

    bool LoadLayout(EditorLayoutData& data) {
        try {
            if (!std::filesystem::exists(layoutFilePath())) return false;

            std::ifstream fin(layoutFilePath());
            if (!fin.is_open()) return false;

            data.dockNodes.clear();
            data.tabs.clear();
            data.preferences.clear();

            std::string line;
            while (std::getline(fin, line)) {
                if (line.empty()) continue;

                std::istringstream iss(line);
                std::string kind;
                iss >> kind;

                if (kind == "DOCK") {
                    EditorLayoutData::DockNodeData node;
                    int visibleInt = 1;
                    iss >> std::quoted(node.id) >> node.x >> node.y >> node.w >> node.h >> visibleInt;
                    if (!iss.fail()) {
                        node.visible = (visibleInt != 0);
                        data.dockNodes.push_back(node);
                    }
                } else if (kind == "TAB") {
                    EditorLayoutData::TabData tab;
                    int activeInt = 0;
                    iss >> std::quoted(tab.id) >> activeInt;
                    if (!iss.fail()) {
                        tab.active = (activeInt != 0);
                        data.tabs.push_back(tab);
                    }
                } else if (kind == "PREF") {
                    std::string key;
                    std::string value;
                    iss >> std::quoted(key) >> std::quoted(value);
                    if (!iss.fail()) {
                        data.preferences[key] = value;
                    }
                }
            }

            return fin.good() || fin.eof();
        } catch (...) {
            return false;
        }
    }

private:
    std::string projectRoot;

    std::string layoutFilePath() const {
        return projectRoot + "/.lightning/editor-layout.data";
    }
};

} // namespace LightningEngine