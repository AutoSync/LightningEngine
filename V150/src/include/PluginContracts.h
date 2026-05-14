// PluginContracts.h — Base contracts for plugin metadata and lifecycle.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace LightningEngine {

enum class PluginScope {
    Global,
    Project
};

enum class PluginPermission : std::uint32_t {
    None      = 0,
    FileSystem= 1u << 0,
    Network   = 1u << 1,
    Device    = 1u << 2,
    ScriptApi = 1u << 3,
    EditorUI  = 1u << 4
};

inline PluginPermission operator|(PluginPermission a, PluginPermission b)
{
    return static_cast<PluginPermission>(
        static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
}

inline bool HasPermission(PluginPermission mask, PluginPermission value)
{
    return (static_cast<std::uint32_t>(mask) & static_cast<std::uint32_t>(value)) != 0u;
}

struct PluginManifest {
    std::string id;
    std::string name;
    std::string version;
    std::string engineVersionMin;
    std::string engineVersionMax;
    PluginScope scope = PluginScope::Project;
    std::string category;
    std::string subcategory;
    std::vector<std::string> dependencies;
    PluginPermission permissions = PluginPermission::None;
    bool enabledByDefault = true;

    std::string entryNative;
    std::string entryCSharp;
    std::string entryIgnite;
};

class IEnginePlugin {
public:
    virtual ~IEnginePlugin() = default;

    virtual const PluginManifest& Manifest() const = 0;
    virtual bool OnLoad() = 0;
    virtual void OnRegister() = 0;
    virtual void OnActivate() = 0;
    virtual void OnDeactivate() = 0;
    virtual void OnUnload() = 0;
};

// New: Editor UI Plugin contract for interface extensibility
class IEditorUIPlugin : public IEnginePlugin {
public:
    virtual ~IEditorUIPlugin() = default;

    // UI-specific lifecycle
    virtual bool OnLoadUI() = 0;
    virtual void OnRegisterPanels() = 0;
    virtual void OnRegisterMenus() = 0;
    virtual void OnSaveLayout() = 0;
    virtual void OnRestoreLayout() = 0;
    virtual void OnUnloadUI() = 0;
};

} // namespace LightningEngine
