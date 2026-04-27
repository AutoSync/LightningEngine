// TypeRegistry.h — Lightweight reflection registry (skeleton).
//
// Manual registration only. No heavy macro magic. Used for serialization,
// editor inspector generation, and language bindings (Rust/JS) over the
// Tauri bridge. See docs/developers/04-reflection.md.
//
// Status: skeleton API. Not consumed by core systems yet — safe to evolve.
#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace LightningEngine {

struct FieldInfo {
    std::string name;
    std::string typeName;
    std::size_t offset = 0;
    std::size_t size   = 0;
};

struct TypeInfo {
    std::string                name;
    std::size_t                size = 0;
    std::vector<FieldInfo>     fields;
    std::function<void*()>     create;
    std::function<void(void*)> destroy;
};

class TypeRegistry {
public:
    static TypeRegistry& Get()
    {
        static TypeRegistry instance;
        return instance;
    }

    // Builder returned by Register<T>() to chain .Field(...) calls.
    template <typename T>
    class Builder {
    public:
        Builder(TypeInfo* info) : info_(info) {}

        template <typename M>
        Builder& Field(const std::string& name, M T::* member)
        {
            FieldInfo f;
            f.name   = name;
            f.size   = sizeof(M);
            // Compute member offset using a null-pointer trick that avoids UB
            // through offsetof macro restrictions on non-standard-layout types.
            f.offset = reinterpret_cast<std::size_t>(
                &(reinterpret_cast<T*>(0)->*member));
            f.typeName = ""; // populated by specialization sites if needed
            info_->fields.push_back(std::move(f));
            return *this;
        }

    private:
        TypeInfo* info_;
    };

    template <typename T>
    Builder<T> Register(const std::string& name)
    {
        TypeInfo info;
        info.name    = name;
        info.size    = sizeof(T);
        info.create  = []() -> void* { return new T(); };
        info.destroy = [](void* p) { delete static_cast<T*>(p); };
        auto [it, inserted] = types_.emplace(name, std::move(info));
        return Builder<T>(&it->second);
    }

    const TypeInfo* Find(const std::string& name) const
    {
        auto it = types_.find(name);
        return it != types_.end() ? &it->second : nullptr;
    }

    std::vector<std::string> ListNames() const
    {
        std::vector<std::string> out;
        out.reserve(types_.size());
        for (const auto& kv : types_) out.push_back(kv.first);
        return out;
    }

private:
    std::unordered_map<std::string, TypeInfo> types_;
};

// Convenience macro for the common "register and chain fields" pattern.
//
//   LE_REGISTER_TYPE(Health)
//       .Field("current", &Health::current)
//       .Field("max",     &Health::max);
#define LE_REGISTER_TYPE(T) \
    ::LightningEngine::TypeRegistry::Get().Register<T>(#T)

} // namespace LightningEngine
