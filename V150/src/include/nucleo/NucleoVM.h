// NucleoVM.h — Stack-based bytecode virtual machine.
// Executes CompiledClass methods. Supports native function call-backs.
#pragma once
#include "NucleoBytecode.h"
#include <vector>
#include <string>
#include <unordered_map>

namespace Nucleo {

// ── Call frame ────────────────────────────────────────────────────────────────
struct CallFrame {
    const Chunk*  chunk     = nullptr;
    size_t        ip        = 0;     // instruction pointer (index into chunk->code)
    size_t        stackBase = 0;     // index of first local in vm.stack
    ObjectRef     self;              // "this" object for the current method
};

// ── VM result ─────────────────────────────────────────────────────────────────
struct VMResult {
    bool        ok    = true;
    Value       value;              // return value of the top-level call
    std::string error;
};

// ── Virtual machine ───────────────────────────────────────────────────────────
class VM {
public:
    // Attach the compiled program. Must be called before Execute.
    void SetProgram(CompiledProgram* prog) { program = prog; }

    // Call a method by name on the given object instance.
    // Pass an empty method name to call the constructor if any.
    VMResult Call(ObjectRef obj, const std::string& method,
                  const std::vector<Value>& args = {});

    // Create a new script object of the named class (runs field defaults).
    ObjectRef Instantiate(const std::string& className);

    // Global variable store (shared across all script instances).
    std::unordered_map<std::string, Value> globals;

    const std::string& LastError() const { return lastError; }

private:
    CompiledProgram*   program   = nullptr;
    std::vector<Value> stack;
    std::vector<CallFrame> frames;
    std::string        lastError;

    void        push(Value v);
    Value       pop();
    Value&      top();

    VMResult    run();

    bool        callMethod(ObjectRef obj, const std::string& name,
                           int argc, bool pushNull = false);
    bool        callNative(int nativeIdx, int argc);

    const CompiledClass*  findClass (const std::string& n) const;
    const CompiledMethod* findMethod(const CompiledClass* cls,
                                     const std::string& name) const;

    void err(const std::string& msg);
};

} // namespace Nucleo
