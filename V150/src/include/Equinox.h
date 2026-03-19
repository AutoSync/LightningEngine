// Equinox.h — Base material system (MaterialInstance).
//
// A MaterialInstance wraps a SparkShader and holds a typed parameter map
// (floats, colors, textures). Call Apply() before SparkShader::Process()
// to push parameters to the shader.
//
// This is the runtime foundation of the Equinox material/shader-composer
// system. The full Material Graph editor is a future feature.
//
// Usage:
//   MaterialInstance mat;
//   mat.shader = &mySparkShader;
//   mat.SetColor("Tint",      { 1.f, 0.5f, 0.5f, 1.f });
//   mat.SetFloat("Intensity", 0.8f);
//   mat.SetTexture("Albedo",  &myTexture);
//
//   // Before rendering the post-process pass:
//   mat.Apply(renderer);
//   mat.shader->Process(renderer, srcFramebuffer, dstFramebuffer);
//
// MaterialLibrary: a named collection of MaterialInstances.
//   MaterialLibrary lib;
//   lib.Add("fire", fireMat);
//   MaterialInstance* m = lib.Get("fire");
#pragma once
#include <string>
#include <unordered_map>
#include "Spark.h"
#include "Texture.h"
#include "Renderer.h"

namespace LightningEngine {

// ── Color (linear float RGBA) ─────────────────────────────────────────────────

struct LinearColor {
    float r = 1.f, g = 1.f, b = 1.f, a = 1.f;

    LinearColor() = default;
    LinearColor(float r, float g, float b, float a = 1.f)
        : r(r), g(g), b(b), a(a) {}

    // Convert 0-255 bytes to linear [0,1].
    static LinearColor FromByte(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255)
    {
        return { r / 255.f, g / 255.f, b / 255.f, a / 255.f };
    }
};

// ── MaterialInstance ──────────────────────────────────────────────────────────

class MaterialInstance {
public:
    SparkShader* shader = nullptr;   // not owned — shared shader template

    // ── Setters ───────────────────────────────────────────────────────────────

    void SetFloat  (const std::string& name, float value)
    { floats[name] = value; }

    void SetColor  (const std::string& name, const LinearColor& c)
    { colors[name] = c; }

    void SetTexture(const std::string& name, Texture* tex)
    { textures[name] = tex; }

    // ── Getters ───────────────────────────────────────────────────────────────

    float GetFloat(const std::string& name, float def = 0.f) const
    {
        auto it = floats.find(name);
        return (it != floats.end()) ? it->second : def;
    }

    LinearColor GetColor(const std::string& name,
                         LinearColor def = { 1.f,1.f,1.f,1.f }) const
    {
        auto it = colors.find(name);
        return (it != colors.end()) ? it->second : def;
    }

    Texture* GetTexture(const std::string& name) const
    {
        auto it = textures.find(name);
        return (it != textures.end()) ? it->second : nullptr;
    }

    // ── Apply ─────────────────────────────────────────────────────────────────
    // Push stored parameters to the SparkShader's known uniform slots.
    // Must be called before SparkShader::Process().
    void Apply(Renderer& /*r*/)
    {
        if (!shader || !shader->IsValid()) return;

        // Map well-known param names to SparkShader setters.
        for (const auto& [name, c] : colors) {
            if (name == "Tint" || name == "_Tint" || name == "tint")
                shader->SetTint(c.r, c.g, c.b, c.a);
            else if (name == "LightColor" || name == "_LightColor") {
                // Get LightPos from floats if available.
                float lx = GetFloat("LightPosX", 0.f);
                float ly = GetFloat("LightPosY", -1.f);
                float lz = GetFloat("LightPosZ", 0.f);
                shader->SetLight(lx, ly, lz, c.r, c.g, c.b);
            }
            else if (name == "FogColor" || name == "_FogColor") {
                float density = GetFloat("FogDensity", 0.02f);
                float start   = GetFloat("FogStart",   10.f);
                float end_    = GetFloat("FogEnd",     100.f);
                shader->SetFog(c.r, c.g, c.b, density, start, end_);
            }
        }

        for (const auto& [name, v] : floats) {
            if (name == "Time" || name == "_Time")
                shader->SetTime(v);
        }
    }

    bool IsValid() const { return shader && shader->IsValid(); }

    void ClearParams()
    {
        floats.clear();
        colors.clear();
        textures.clear();
    }

private:
    std::unordered_map<std::string, float>       floats;
    std::unordered_map<std::string, LinearColor> colors;
    std::unordered_map<std::string, Texture*>    textures;
};

// ── MaterialLibrary ───────────────────────────────────────────────────────────
// A named collection of MaterialInstances.

class MaterialLibrary {
public:
    void Add(const std::string& name, const MaterialInstance& mat)
    { library[name] = mat; }

    MaterialInstance* Get(const std::string& name)
    {
        auto it = library.find(name);
        return (it != library.end()) ? &it->second : nullptr;
    }

    bool Has(const std::string& name) const
    { return library.count(name) > 0; }

    void Remove(const std::string& name) { library.erase(name); }
    void Clear()  { library.clear(); }

    int Count() const { return (int)library.size(); }

    // Iterate all materials.
    std::unordered_map<std::string, MaterialInstance>& GetAll() { return library; }

private:
    std::unordered_map<std::string, MaterialInstance> library;
};

} // namespace LightningEngine
