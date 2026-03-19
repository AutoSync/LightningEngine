// NucleoValue.h — Runtime value type for the Núcleo VM.
// Dynamic-typed tagged union: Null | Int | Float | Bool | String | Object.
#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include <vector>

namespace Nucleo {

// Forward-declare NucleoObject so Value can hold an ObjectRef.
struct NucleoObject;
using ObjectRef = std::shared_ptr<NucleoObject>;

// ── Runtime value ─────────────────────────────────────────────────────────────
struct Value {
    enum class T { Null, Int, Float, Bool, String, Object };
    T type = T::Null;

    union {
        int   ival = 0;
        float fval;
        bool  bval;
    };
    std::string sval;
    ObjectRef   obj;

    // Constructors
    static Value Null()          { Value v; v.type = T::Null;   return v; }
    static Value Int(int i)      { Value v; v.type = T::Int;    v.ival = i;  return v; }
    static Value Float(float f)  { Value v; v.type = T::Float;  v.fval = f;  return v; }
    static Value Bool(bool b)    { Value v; v.type = T::Bool;   v.bval = b;  return v; }
    static Value String(const std::string& s) { Value v; v.type = T::String; v.sval = s; return v; }
    static Value Object(ObjectRef o) { Value v; v.type = T::Object; v.obj = std::move(o); return v; }

    // Coercions — implementations below (after NucleoObject is fully defined)
    bool   toBool()   const;
    float  toFloat()  const;
    int    toInt()    const;
    std::string toString() const;

    bool isNull()   const { return type == T::Null;   }
    bool isInt()    const { return type == T::Int;    }
    bool isFloat()  const { return type == T::Float;  }
    bool isNumber() const { return type == T::Int || type == T::Float; }
    bool isBool()   const { return type == T::Bool;   }
    bool isString() const { return type == T::String; }
    bool isObject() const { return type == T::Object; }

    bool operator==(const Value& o) const;
    bool operator!=(const Value& o) const { return !(*this == o); }
};

// Inline implementations that do NOT need NucleoObject's internals
inline bool Value::toBool() const
{
    switch (type) {
    case T::Null:   return false;
    case T::Int:    return ival != 0;
    case T::Float:  return fval != 0.f;
    case T::Bool:   return bval;
    case T::String: return !sval.empty();
    case T::Object: return obj != nullptr;
    }
    return false;
}

inline float Value::toFloat() const
{
    if (type == T::Float)  return fval;
    if (type == T::Int)    return (float)ival;
    if (type == T::Bool)   return bval ? 1.f : 0.f;
    return 0.f;
}

inline int Value::toInt() const
{
    if (type == T::Int)    return ival;
    if (type == T::Float)  return (int)fval;
    if (type == T::Bool)   return bval ? 1 : 0;
    return 0;
}

// ── Runtime object (script class instance) ───────────────────────────────────
// Full definition must come BEFORE toString() which accesses obj->className.
struct NucleoObject {
    std::string className;
    std::unordered_map<std::string, Value> fields;

    // Optional: bound C++ node pointer (set by NucleoScriptComponent)
    void* boundNode = nullptr;

    Value& Field(const std::string& name)
    {
        auto it = fields.find(name);
        if (it != fields.end()) return it->second;
        fields[name] = Value::Null();
        return fields[name];
    }
};

// Inline implementations that need NucleoObject to be fully defined
inline std::string Value::toString() const
{
    switch (type) {
    case T::Null:   return "null";
    case T::Int:    return std::to_string(ival);
    case T::Float:  return std::to_string(fval);
    case T::Bool:   return bval ? "true" : "false";
    case T::String: return sval;
    case T::Object: return obj ? "[" + obj->className + "]" : "[null]";
    }
    return "";
}

inline bool Value::operator==(const Value& o) const
{
    if (type != o.type) {
        if (isNumber() && o.isNumber()) return toFloat() == o.toFloat();
        return false;
    }
    switch (type) {
    case T::Null:   return true;
    case T::Int:    return ival == o.ival;
    case T::Float:  return fval == o.fval;
    case T::Bool:   return bval == o.bval;
    case T::String: return sval == o.sval;
    case T::Object: return obj  == o.obj;
    }
    return false;
}

// ── Native function signature ────────────────────────────────────────────────
using NativeFn = std::function<Value(const std::vector<Value>&)>;

struct NativeFunc {
    std::string name;
    NativeFn    fn;
};

} // namespace Nucleo
