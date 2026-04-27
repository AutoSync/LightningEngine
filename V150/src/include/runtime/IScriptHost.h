// IScriptHost.h — Pure interface for the Lightning Engine scripting runtime (Nucleo).
//
// Plugins can start/stop scripts, evaluate expressions, and register native
// functions without depending on the full Nucleo pipeline headers.
//
// See: src/include/Nucleo.h + src/include/nucleo/* (concrete implementation).
// See: docs/developers/01-architecture.md
#pragma once

#include <functional>
#include <string>
#include <vector>

namespace LightningEngine {

// Variant value type exchanged between native code and scripts.
// Mirrors Nucleo::Value semantics without exposing Nucleo headers.
struct ScriptValue {
    enum class Type { Null, Bool, Number, String, Object } type = Type::Null;

    bool        boolVal   = false;
    double      numVal    = 0.0;
    std::string strVal;
    void*       objRef    = nullptr; // opaque Nucleo ObjectRef
};

// Signature for native functions callable from scripts.
using NativeScriptFn = std::function<
    ScriptValue(const std::vector<ScriptValue>& args)>;

class IScriptHost {
public:
    virtual ~IScriptHost() = default;

    // ── Lifecycle ────────────────────────────────────────────────────────
    // Compile and load a script file. Returns false + fills error on failure.
    virtual bool   LoadScript(const std::string& path,
                              std::string& outError)       = 0;

    // ── Instance management ──────────────────────────────────────────────
    // Create an instance of a script class. Returns opaque id (0 = failed).
    virtual uint32_t Instantiate(const std::string& className) = 0;
    // Destroy a script instance.
    virtual void     DestroyInstance(uint32_t instanceId)      = 0;

    // ── Method calls ─────────────────────────────────────────────────────
    // Call a method on an instance. Returns the script return value.
    virtual ScriptValue Call(uint32_t instanceId,
                             const std::string& method,
                             const std::vector<ScriptValue>& args = {}) = 0;

    // ── Native bindings ──────────────────────────────────────────────────
    // Register a native C++ function callable from any script.
    virtual void RegisterNative(const std::string& name,
                                NativeScriptFn fn)             = 0;

    // ── Global state ─────────────────────────────────────────────────────
    virtual void         SetGlobal(const std::string& name, ScriptValue val) = 0;
    virtual ScriptValue  GetGlobal(const std::string& name) const            = 0;

    // ── Error handling ───────────────────────────────────────────────────
    virtual const std::string& LastError() const = 0;
};

} // namespace LightningEngine
