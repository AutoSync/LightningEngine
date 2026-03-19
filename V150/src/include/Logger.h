// Logger.h — Global log buffer for the Lightning Engine editor console.
//
// Usage (engine / script side):
//   #include "Logger.h"
//   Logger::LogInfo("Hello");
//   Logger::LogWarning("Watch out");
//   Logger::LogError("Something failed");
//   Logger::LogScript("script output");
//
// Editor side:
//   if (Logger::Dirty()) { refreshConsole(); Logger::ClearDirty(); }
#pragma once
#include <string>
#include <vector>

namespace Logger {

enum class Level { Info, Warning, Error, Script };

struct Entry {
    Level       level;
    std::string text;
};

// ── Storage ───────────────────────────────────────────────────────────────────
inline std::vector<Entry>& GetEntries() {
    static std::vector<Entry> s;
    return s;
}

inline bool& _dirty() {
    static bool d = false;
    return d;
}

inline bool  Dirty()      { return _dirty(); }
inline void  ClearDirty() { _dirty() = false; }

// ── Write ─────────────────────────────────────────────────────────────────────
inline void Log(Level level, const std::string& msg) {
    auto& entries = GetEntries();
    entries.push_back({ level, msg });
    if (entries.size() > 2000)
        entries.erase(entries.begin(), entries.begin() + 500);
    _dirty() = true;
}

inline void LogInfo   (const std::string& msg) { Log(Level::Info,    msg); }
inline void LogWarning(const std::string& msg) { Log(Level::Warning, msg); }
inline void LogError  (const std::string& msg) { Log(Level::Error,   msg); }
inline void LogScript (const std::string& msg) { Log(Level::Script,  msg); }

inline void Clear() {
    GetEntries().clear();
    _dirty() = true;
}

} // namespace Logger
