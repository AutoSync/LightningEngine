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
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include "Spark.h"
#include "Texture.h"
#include "Renderer.h"

namespace LightningEngine {

enum class EquinoxAssetType {
    Unknown,
    ShaderComposer,
    TextureGenerator,
};

struct EquinoxDocumentInfo {
    EquinoxAssetType type = EquinoxAssetType::Unknown;
    std::string path;
    std::string name;
    std::string displayName;
    std::string extension;
    std::string stage = "fragment";
    std::string output;
    int nodeCount = 0;
    int parameterCount = 0;
    int outputCount = 0;
    bool exists = false;
    bool textual = false;
};

class EquinoxFileManager {
public:
    static std::string ToLowerCopy(std::string text)
    {
        std::transform(text.begin(), text.end(), text.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return text;
    }

    static std::string TrimCopy(const std::string& text)
    {
        std::size_t start = 0;
        while (start < text.size() && std::isspace(static_cast<unsigned char>(text[start]))) ++start;
        std::size_t end = text.size();
        while (end > start && std::isspace(static_cast<unsigned char>(text[end - 1]))) --end;
        return text.substr(start, end - start);
    }

    static EquinoxAssetType DetectAssetTypeFromExtension(const std::string& extension)
    {
        const std::string ext = ToLowerCopy(extension);
        if (ext == ".equinox" || ext == ".lmat" || ext == ".lmatfunc" ||
            ext == ".lmatinst" || ext == ".lmatlayer") {
            return EquinoxAssetType::ShaderComposer;
        }
        if (ext == ".etexgen") return EquinoxAssetType::TextureGenerator;
        return EquinoxAssetType::Unknown;
    }

    static EquinoxAssetType DetectAssetTypeFromPath(const std::string& path)
    {
        return DetectAssetTypeFromExtension(std::filesystem::path(path).extension().string());
    }

    static bool IsEquinoxExtension(const std::string& extension)
    {
        return DetectAssetTypeFromExtension(extension) != EquinoxAssetType::Unknown;
    }

    static bool IsEquinoxAssetPath(const std::string& path)
    {
        return DetectAssetTypeFromPath(path) != EquinoxAssetType::Unknown;
    }

    static const char* DisplayName(EquinoxAssetType type)
    {
        switch (type) {
        case EquinoxAssetType::ShaderComposer: return "Equinox Shader Composer";
        case EquinoxAssetType::TextureGenerator: return "Equinox Texture Generator";
        default: return "Equinox Asset";
        }
    }

    static const char* DefaultStem(EquinoxAssetType type)
    {
        switch (type) {
        case EquinoxAssetType::ShaderComposer: return "NewEquinoxComposer";
        case EquinoxAssetType::TextureGenerator: return "NewTextureGenerator";
        default: return "NewEquinoxAsset";
        }
    }

    static const char* DefaultExtension(EquinoxAssetType type)
    {
        switch (type) {
        case EquinoxAssetType::ShaderComposer: return ".equinox";
        case EquinoxAssetType::TextureGenerator: return ".etexgen";
        default: return "";
        }
    }

    static std::string BuildDefaultDocument(EquinoxAssetType type,
                                            const std::string& assetName = "")
    {
        const std::string safeName = assetName.empty() ? DefaultStem(type) : assetName;

        if (type == EquinoxAssetType::TextureGenerator) {
            return
                "type: texture_generator\n"
                "name: " + safeName + "\n"
                "generator:\n"
                "  width: 1024\n"
                "  height: 1024\n"
                "  format: rgba8\n"
                "  output: " + safeName + ".png\n"
                "graph:\n"
                "  - node: Noise\n"
                "    id: base_noise\n"
                "  - node: Output\n"
                "    id: final_output\n";
        }

        return
            "type: shader_composer\n"
            "name: " + safeName + "\n"
            "shader:\n"
            "  stage: fragment\n"
            "  output: material\n"
            "parameters:\n"
            "  - name: BaseColor\n"
            "    kind: color\n"
            "graph:\n"
            "  - node: TextureSample\n"
            "    id: albedo\n"
            "  - node: Output\n"
            "    id: surface\n";
    }

    static bool ReadTextFile(const std::string& path, std::string& outText)
    {
        std::ifstream in(path, std::ios::binary);
        if (!in) return false;

        std::ostringstream buffer;
        buffer << in.rdbuf();
        outText = buffer.str();

        const std::size_t checkLen = std::min<std::size_t>(outText.size(), 2048);
        for (std::size_t i = 0; i < checkLen; ++i) {
            if (outText[i] == '\0') return false;
        }
        return true;
    }

    static bool WriteTextFile(const std::string& path, const std::string& text)
    {
        std::ofstream out(path, std::ios::binary | std::ios::trunc);
        if (!out) return false;
        out << text;
        return static_cast<bool>(out);
    }

    static EquinoxDocumentInfo InspectDocument(const std::string& path,
                                               const std::string& bodyText = "")
    {
        EquinoxDocumentInfo info;
        info.path = path;
        info.extension = ToLowerCopy(std::filesystem::path(path).extension().string());
        info.type = DetectAssetTypeFromExtension(info.extension);
        info.displayName = DisplayName(info.type);
        info.name = std::filesystem::path(path).stem().string();
        info.exists = !path.empty() && std::filesystem::exists(std::filesystem::path(path));

        std::string text = bodyText;
        if (text.empty() && info.exists) {
            if (!ReadTextFile(path, text)) return info;
        }

        if (text.empty()) return info;
        info.textual = true;

        auto extractValue = [](const std::string& line) {
            const std::size_t colon = line.find(':');
            if (colon == std::string::npos) return std::string();
            return TrimCopy(line.substr(colon + 1));
        };

        std::istringstream stream(text);
        std::string line;
        while (std::getline(stream, line)) {
            const std::string trimmed = TrimCopy(line);
            if (trimmed.empty() || trimmed[0] == '#') continue;

            const std::string lower = ToLowerCopy(trimmed);
            if (lower.rfind("name:", 0) == 0) {
                const std::string value = extractValue(trimmed);
                if (!value.empty()) info.name = value;
            } else if (lower.find("stage:") != std::string::npos) {
                const std::string value = extractValue(trimmed);
                if (!value.empty()) info.stage = value;
            } else if (lower.find("output:") != std::string::npos) {
                const std::string value = extractValue(trimmed);
                if (!value.empty()) info.output = value;
                ++info.outputCount;
            }

            if (lower.rfind("- node:", 0) == 0 || lower.find(" node:") != std::string::npos) {
                ++info.nodeCount;
            } else if (lower.rfind("- ", 0) == 0 && (lower.find("id:") != std::string::npos ||
                       lower.find("kind:") != std::string::npos || lower.find("type:") != std::string::npos)) {
                ++info.nodeCount;
            }

            if (lower.find("parameter") != std::string::npos || lower.find("uniform:") != std::string::npos ||
                lower.find("param:") != std::string::npos || lower.find("kind:") != std::string::npos) {
                ++info.parameterCount;
            }
        }

        if (info.output.empty()) {
            info.output = (info.type == EquinoxAssetType::TextureGenerator) ? "generated.png" : "material";
        }
        return info;
    }
};

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
