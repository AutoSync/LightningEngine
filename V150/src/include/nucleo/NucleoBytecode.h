// NucleoBytecode.h — Opcodes and Chunk (compiled bytecode unit).
// Each Chunk represents a compiled method body.
// The VM executes chunks on a value stack with a call-frame stack.
#pragma once
#include "NucleoValue.h"
#include <vector>
#include <string>
#include <cstdint>

namespace Nucleo {

// ── Opcodes ───────────────────────────────────────────────────────────────────
enum class Op : uint8_t {
    // Constants / literals
    PUSH_CONST,         // arg16: constant pool index
    PUSH_NULL,
    PUSH_TRUE,
    PUSH_FALSE,

    // Stack manipulation
    POP,
    DUP,

    // Local variables
    LOAD_LOCAL,         // arg16: slot
    STORE_LOCAL,        // arg16: slot

    // Instance fields (on "this" object)
    LOAD_THIS_FIELD,    // arg16: name pool index
    STORE_THIS_FIELD,   // arg16: name pool index

    // Field on arbitrary object (object on stack)
    LOAD_FIELD,         // arg16: name pool index   [obj] → [val]
    STORE_FIELD,        // arg16: name pool index   [obj, val] → []

    // Global variables (static / engine objects)
    LOAD_GLOBAL,        // arg16: name pool index
    STORE_GLOBAL,       // arg16: name pool index

    // Arithmetic
    ADD, SUB, MUL, DIV, MOD,
    NEGATE,

    // Comparison
    EQ, NEQ, LT, LTE, GT, GTE,

    // Logical
    LAND, LOR, LNOT,

    // String concat (auto-coerce both sides)
    CONCAT,

    // Control flow
    JMP,                // arg16: signed relative offset (from next instruction)
    JMP_FALSE,          // pop top, jump if false
    JMP_TRUE,           // pop top, jump if true

    // Calls
    CALL_METHOD,        // arg16: argc   arg16: name_idx   obj on stack below args
    CALL_NATIVE,        // arg16: native_index   arg16: argc
    CALL_CHUNK,         // arg16: chunk_index   arg16: argc   (static/free function)

    // Return
    RETURN,             // return top of stack
    RETURN_VOID,        // push null then return

    // Object creation
    NEW_OBJ,            // arg16: className name_idx   → ObjectRef on stack

    // Increment helpers
    INC_LOCAL,          // arg16: slot  (adds 1 in place)
    DEC_LOCAL,          // arg16: slot

    HALT
};

// ── Chunk ─────────────────────────────────────────────────────────────────────
struct Chunk {
    std::string            name;          // method/function name (debug)
    std::vector<uint8_t>   code;          // bytecode
    std::vector<Value>     constants;     // constant pool
    std::vector<std::string> names;       // name pool (field/global/method names)
    std::vector<int>       lines;         // line numbers parallel to code[]

    // Emission helpers
    size_t  emit(Op op)            { code.push_back((uint8_t)op); lines.push_back(curLine); return code.size()-1; }
    void    emitArg16(uint16_t v)  { code.push_back(v & 0xFF); code.push_back((v >> 8) & 0xFF); }

    size_t  emitJmp(Op op)         { emit(op); size_t off = code.size(); emitArg16(0); return off; }
    void    patchJmp(size_t off)   {
        int delta = (int)code.size() - (int)(off + 2);
        code[off]   = (uint8_t)(delta & 0xFF);
        code[off+1] = (uint8_t)((delta >> 8) & 0xFF);
    }

    uint16_t addConst(Value v)  { constants.push_back(std::move(v)); return (uint16_t)(constants.size()-1); }
    uint16_t addName(const std::string& s) {
        for (size_t i = 0; i < names.size(); ++i)
            if (names[i] == s) return (uint16_t)i;
        names.push_back(s);
        return (uint16_t)(names.size()-1);
    }

    // Decode a 16-bit arg starting at offset `off`
    uint16_t readU16(size_t off) const
    { return (uint16_t)code[off] | ((uint16_t)code[off+1] << 8); }

    int16_t readS16(size_t off) const
    { return (int16_t)readU16(off); }

    int curLine = 0; // set by compiler before emitting
};

// ── Compiled class ────────────────────────────────────────────────────────────
struct CompiledMethod {
    std::string         name;
    std::vector<std::string> paramNames;
    int                 localCount = 0;
    Chunk               chunk;
};

struct CompiledClass {
    std::string                       name;
    std::string                       baseName;
    std::vector<std::string>          fieldNames;
    std::vector<Value>                fieldDefaults;
    std::vector<CompiledMethod>       methods;

    const CompiledMethod* FindMethod(const std::string& n) const {
        for (auto& m : methods) if (m.name == n) return &m;
        return nullptr;
    }
};

struct CompiledProgram {
    std::vector<CompiledClass>  classes;
    std::vector<NativeFunc>     natives; // registered by NucleoAPI

    const CompiledClass* FindClass(const std::string& n) const {
        for (auto& c : classes) if (c.name == n) return &c;
        return nullptr;
    }
};

} // namespace Nucleo
