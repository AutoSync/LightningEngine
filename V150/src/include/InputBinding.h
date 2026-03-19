// InputBinding.h — Named action → key/button mapping with runtime rebinding.
//
// Define named actions ("Jump", "MoveLeft") mapped to SDL scancodes.
// Query by action name instead of hardcoded keys, then rebind at runtime
// without touching gameplay code.
//
// Usage:
//   InputBinding bindings;
//   bindings.Define("Jump",     SDL_SCANCODE_SPACE, SDL_SCANCODE_UNKNOWN);
//   bindings.Define("MoveLeft", SDL_SCANCODE_A,     SDL_SCANCODE_LEFT);
//   bindings.Define("Fire",     SDL_SCANCODE_LCTRL, SDL_SCANCODE_UNKNOWN,
//                               SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER);
//
//   // In Update():
//   if (bindings.IsPressed("Jump", inputManager))
//       player.Jump();
//
//   // Rebind at runtime (e.g. from a settings menu):
//   bindings.Rebind("Jump", SDL_SCANCODE_RETURN);
//
//   // Serialize to game.ini [Bindings] section:
//   bindings.SaveToIni(ini);
//   bindings.LoadFromIni(ini);
#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <SDL3/SDL.h>
#include "InputManager.h"
#include "ProjectConfig.h"

namespace LightningEngine {

struct Binding {
    std::string  action;
    SDL_Scancode primary   = SDL_SCANCODE_UNKNOWN;
    SDL_Scancode secondary = SDL_SCANCODE_UNKNOWN;
    int          gamepadBtn = -1;   // SDL_GamepadButton value, or -1 = none
};

class InputBinding {
public:
    // ── Definition ───────────────────────────────────────────────────────────

    void Define(const std::string& action,
                SDL_Scancode primary,
                SDL_Scancode secondary = SDL_SCANCODE_UNKNOWN,
                int gamepadBtn = -1)
    {
        bindings[action] = { action, primary, secondary, gamepadBtn };
    }

    void Undefine(const std::string& action)
    {
        bindings.erase(action);
    }

    bool HasAction(const std::string& action) const
    {
        return bindings.count(action) > 0;
    }

    // Returns a list of all defined action names.
    std::vector<std::string> GetActions() const
    {
        std::vector<std::string> out;
        out.reserve(bindings.size());
        for (const auto& [k, _] : bindings) out.push_back(k);
        return out;
    }

    // ── Rebinding ─────────────────────────────────────────────────────────────

    void Rebind(const std::string& action, SDL_Scancode newPrimary,
                SDL_Scancode newSecondary = SDL_SCANCODE_UNKNOWN)
    {
        auto it = bindings.find(action);
        if (it != bindings.end()) {
            it->second.primary   = newPrimary;
            it->second.secondary = newSecondary;
        }
    }

    void RebindGamepad(const std::string& action, int gamepadBtn)
    {
        auto it = bindings.find(action);
        if (it != bindings.end())
            it->second.gamepadBtn = gamepadBtn;
    }

    const Binding* GetBinding(const std::string& action) const
    {
        auto it = bindings.find(action);
        return (it != bindings.end()) ? &it->second : nullptr;
    }

    // ── Queries ───────────────────────────────────────────────────────────────

    bool IsDown(const std::string& action, const InputManager& im) const
    {
        const Binding* b = GetBinding(action);
        if (!b) return false;
        bool kb = (b->primary   != SDL_SCANCODE_UNKNOWN && im.IsKeyDown(b->primary))
               || (b->secondary != SDL_SCANCODE_UNKNOWN && im.IsKeyDown(b->secondary));
        bool gp = (b->gamepadBtn >= 0 && im.IsGamepadButtonDown(b->gamepadBtn));
        return kb || gp;
    }

    bool IsPressed(const std::string& action, const InputManager& im) const
    {
        const Binding* b = GetBinding(action);
        if (!b) return false;
        bool kb = (b->primary   != SDL_SCANCODE_UNKNOWN && im.IsKeyPressed(b->primary))
               || (b->secondary != SDL_SCANCODE_UNKNOWN && im.IsKeyPressed(b->secondary));
        bool gp = (b->gamepadBtn >= 0 && im.IsGamepadButtonPressed(b->gamepadBtn));
        return kb || gp;
    }

    bool IsReleased(const std::string& action, const InputManager& im) const
    {
        const Binding* b = GetBinding(action);
        if (!b) return false;
        bool kb = (b->primary   != SDL_SCANCODE_UNKNOWN && im.IsKeyReleased(b->primary))
               || (b->secondary != SDL_SCANCODE_UNKNOWN && im.IsKeyReleased(b->secondary));
        bool gp = (b->gamepadBtn >= 0 && im.IsGamepadButtonReleased(b->gamepadBtn));
        return kb || gp;
    }

    // ── Serialization ─────────────────────────────────────────────────────────
    // Writes to/reads from an IniFile under section "[Bindings]".
    // Key = action name, value = "primary;secondary;gamepadBtn" (scancode ints).

    void SaveToIni(IniFile& ini) const
    {
        for (const auto& [name, b] : bindings) {
            std::string val = std::to_string((int)b.primary)   + ";"
                            + std::to_string((int)b.secondary) + ";"
                            + std::to_string(b.gamepadBtn);
            ini.Set("Bindings", name, val);
        }
    }

    void LoadFromIni(const IniFile& ini)
    {
        // Iterate existing actions — load only if the key exists in the ini.
        for (auto& [name, b] : bindings) {
            std::string val = ini.Get("Bindings", name, "");
            if (val.empty()) continue;

            // Parse "primary;secondary;gamepadBtn"
            int p = SDL_SCANCODE_UNKNOWN, s = SDL_SCANCODE_UNKNOWN, g = -1;
            if (sscanf(val.c_str(), "%d;%d;%d", &p, &s, &g) >= 1) {
                b.primary    = (SDL_Scancode)p;
                b.secondary  = (SDL_Scancode)s;
                b.gamepadBtn = g;
            }
        }
    }

    // Clear all bindings.
    void Clear() { bindings.clear(); }

private:
    std::unordered_map<std::string, Binding> bindings;
};

} // namespace LightningEngine
