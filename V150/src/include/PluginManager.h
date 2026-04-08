// PluginManager.h — In-memory plugin lifecycle manager (phase 1).
#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "PluginContracts.h"

namespace LightningEngine {

enum class PluginState {
    Discovered,
    Loaded,
    Registered,
    Active,
    Disabled,
    Failed
};

class PluginManager {
public:
    struct Entry {
        std::shared_ptr<IEnginePlugin> plugin;
        PluginState state = PluginState::Discovered;
        std::string lastError;
    };

    bool AddPlugin(const std::shared_ptr<IEnginePlugin>& plugin)
    {
        if (!plugin) return false;
        const std::string id = plugin->Manifest().id;
        if (id.empty()) return false;
        if (entries.find(id) != entries.end()) return false;
        entries.emplace(id, Entry{ plugin, PluginState::Discovered, {} });
        return true;
    }

    bool Load(const std::string& id)
    {
        Entry* e = get(id);
        if (!e || !e->plugin) return false;
        if (e->state == PluginState::Disabled) return false;
        if (!e->plugin->OnLoad()) {
            e->state = PluginState::Failed;
            e->lastError = "OnLoad returned false";
            return false;
        }
        e->state = PluginState::Loaded;
        e->lastError.clear();
        return true;
    }

    bool Register(const std::string& id)
    {
        Entry* e = get(id);
        if (!e || !e->plugin) return false;
        if (e->state != PluginState::Loaded && e->state != PluginState::Registered) return false;
        e->plugin->OnRegister();
        e->state = PluginState::Registered;
        return true;
    }

    bool Activate(const std::string& id)
    {
        Entry* e = get(id);
        if (!e || !e->plugin) return false;
        if (e->state != PluginState::Registered && e->state != PluginState::Active) return false;
        e->plugin->OnActivate();
        e->state = PluginState::Active;
        return true;
    }

    bool Deactivate(const std::string& id)
    {
        Entry* e = get(id);
        if (!e || !e->plugin) return false;
        if (e->state != PluginState::Active) return false;
        e->plugin->OnDeactivate();
        e->state = PluginState::Registered;
        return true;
    }

    bool Unload(const std::string& id)
    {
        Entry* e = get(id);
        if (!e || !e->plugin) return false;
        if (e->state == PluginState::Active) {
            e->plugin->OnDeactivate();
        }
        e->plugin->OnUnload();
        e->state = PluginState::Discovered;
        return true;
    }

    bool SetEnabled(const std::string& id, bool enabled)
    {
        Entry* e = get(id);
        if (!e) return false;
        if (!enabled) {
            e->state = PluginState::Disabled;
            return true;
        }
        if (e->state == PluginState::Disabled) {
            e->state = PluginState::Discovered;
        }
        return true;
    }

    const Entry* Find(const std::string& id) const
    {
        auto it = entries.find(id);
        return (it != entries.end()) ? &it->second : nullptr;
    }

    std::vector<std::string> ListIds() const
    {
        std::vector<std::string> ids;
        ids.reserve(entries.size());
        for (const auto& kv : entries) ids.push_back(kv.first);
        return ids;
    }

private:
    Entry* get(const std::string& id)
    {
        auto it = entries.find(id);
        return (it != entries.end()) ? &it->second : nullptr;
    }

    std::unordered_map<std::string, Entry> entries;
};

} // namespace LightningEngine
