// ProjectConfig.h — Project, Game and Editor configuration.
// Each struct maps to an INI file (project.ini / game.ini / editor.ini).
// IniFile is a minimal INI reader/writer with no external dependencies.
#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>

namespace LightningEngine {

// ── Minimal INI reader / writer ───────────────────────────────────────────────
struct IniFile {
    std::unordered_map<std::string,
        std::unordered_map<std::string, std::string>> sections;

    bool Load(const std::string& path)
    {
        std::ifstream f(path);
        if (!f) return false;
        std::string line, section;
        while (std::getline(f, line)) {
            if (line.empty() || line[0] == '#' || line[0] == ';') continue;
            if (line[0] == '[') {
                auto end = line.find(']');
                if (end != std::string::npos)
                    section = line.substr(1, end - 1);
            } else {
                auto eq = line.find('=');
                if (eq != std::string::npos)
                    sections[section][trim(line.substr(0, eq))] =
                        trim(line.substr(eq + 1));
            }
        }
        return true;
    }

    bool Save(const std::string& path) const
    {
        std::ofstream f(path);
        if (!f) return false;
        for (const auto& [sec, kvs] : sections) {
            f << "[" << sec << "]\n";
            for (const auto& [k, v] : kvs)
                f << k << "=" << v << "\n";
            f << "\n";
        }
        return true;
    }

    std::string Get(const std::string& sec, const std::string& key,
                    const std::string& def = "") const
    {
        auto si = sections.find(sec);
        if (si == sections.end()) return def;
        auto ki = si->second.find(key);
        return ki == si->second.end() ? def : ki->second;
    }

    void Set(const std::string& sec, const std::string& key, const std::string& val)
    { sections[sec][key] = val; }

    int  GetInt (const std::string& s, const std::string& k, int  d = 0)    const
    { try { return std::stoi(Get(s, k, std::to_string(d))); } catch(...){ return d; } }

    bool GetBool(const std::string& s, const std::string& k, bool d = false) const
    { std::string v = Get(s, k, d ? "true" : "false"); return v == "true" || v == "1"; }

private:
    static std::string trim(const std::string& s)
    {
        size_t a = s.find_first_not_of(" \t\r\n");
        size_t b = s.find_last_not_of(" \t\r\n");
        return a == std::string::npos ? "" : s.substr(a, b - a + 1);
    }
};

// ── project.ini ───────────────────────────────────────────────────────────────
struct ProjectConfig {
    std::string name      = "Untitled";
    std::string version   = "0.1.0";
    std::string lastScene = "scenes/main.lescene";
    std::string rootPath;   // absolute path to project folder (set by ProjectManager)

    bool Load(const std::string& iniPath)
    {
        IniFile f;
        if (!f.Load(iniPath)) return false;
        name      = f.Get("Project", "Name",      name);
        version   = f.Get("Project", "Version",   version);
        lastScene = f.Get("Project", "LastScene", lastScene);
        return true;
    }

    bool Save(const std::string& iniPath) const
    {
        IniFile f;
        f.Set("Project", "Name",      name);
        f.Set("Project", "Version",   version);
        f.Set("Project", "LastScene", lastScene);
        return f.Save(iniPath);
    }
};

// ── game.ini ──────────────────────────────────────────────────────────────────
struct GameConfig {
    std::string title      = "My Game";
    int         width      = 1280;
    int         height     = 720;
    bool        fullscreen = false;

    bool Load(const std::string& iniPath)
    {
        IniFile f;
        if (!f.Load(iniPath)) return false;
        title      = f.Get    ("Game", "Title",      title);
        width      = f.GetInt ("Game", "Width",      width);
        height     = f.GetInt ("Game", "Height",     height);
        fullscreen = f.GetBool("Game", "Fullscreen", fullscreen);
        return true;
    }

    bool Save(const std::string& iniPath) const
    {
        IniFile f;
        f.Set("Game", "Title",      title);
        f.Set("Game", "Width",      std::to_string(width));
        f.Set("Game", "Height",     std::to_string(height));
        f.Set("Game", "Fullscreen", fullscreen ? "true" : "false");
        return f.Save(iniPath);
    }
};

// ── editor.ini ────────────────────────────────────────────────────────────────
struct EditorConfig {
    std::string theme    = "Dark";
    std::string lastPath = "";

    bool Load(const std::string& iniPath)
    {
        IniFile f;
        if (!f.Load(iniPath)) return false;
        theme    = f.Get("Editor", "Theme",    theme);
        lastPath = f.Get("Editor", "LastPath", lastPath);
        return true;
    }

    bool Save(const std::string& iniPath) const
    {
        IniFile f;
        f.Set("Editor", "Theme",    theme);
        f.Set("Editor", "LastPath", lastPath);
        return f.Save(iniPath);
    }
};

} // namespace LightningEngine
