// NucleoScript.h — Component that loads, compiles, and runs a .spark script file.
//
// Usage:
//   auto* ns = node->AddComponent<NucleoScriptComponent>();
//   ns->SetScript("scripts/PlayerController.spark");
//   // OnAttach was already called — call Reload() if you set the path after attach.
//
// Lifecycle callbacks the script class may define:
//   void OnStart()          — called once when the script is first attached
//   void Update(float dt)   — called every frame with delta time in seconds
//   void OnDestroy()        — called when the component is detached
//
// The script class name must match the file stem (e.g. PlayerController.spark
// should contain "class PlayerController { ... }").
// If the script defines a class whose name differs from the file stem, set
// ns->className explicitly before calling Reload().
#pragma once
#include "../Component.h"
#include "../Logger.h"
#include "NucleoLexer.h"
#include "NucleoParser.h"
#include "NucleoCompiler.h"
#include "NucleoVM.h"
#include "NucleoAPI.h"
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>

namespace LightningEngine
{

class NucleoScriptComponent : public Component
{
public:
    std::string scriptPath; // relative or absolute path to the .spark file
    std::string className;  // override — leave empty to derive from file stem
    std::string lastError;

    // ── Set the script path and reload (convenience) ────────────────────────
    void SetScript(const std::string& path)
    {
        scriptPath = path;
        if (className.empty())
            className = stemFromPath(path);
        Reload();
    }

    // ── Load, compile and start the script. Safe to call at runtime. ────────
    bool Reload()
    {
        lastError.clear();
        instance.reset();

        if (scriptPath.empty()) { lastError = "No script path set"; return false; }

        // 1. Read source
        std::ifstream f(scriptPath);
        if (!f.is_open()) {
            lastError = "Cannot open: " + scriptPath;
            std::cerr << "[NucleoScript] " << lastError << "\n";
            Logger::LogError("[Script] " + lastError);
            return false;
        }
        std::ostringstream ss; ss << f.rdbuf();
        std::string source = ss.str();

        // 2. Lex
        Nucleo::Lexer lexer;
        auto tokens = lexer.Tokenise(source);

        // 3. Parse
        Nucleo::Parser parser;
        Nucleo::Program ast = parser.Parse(tokens);
        if (!parser.Error().empty()) {
            lastError = "Parse error: " + parser.Error();
            std::cerr << "[NucleoScript] " << lastError << "\n";
            Logger::LogError("[Script] " + lastError);
            return false;
        }

        // 4. Compile
        program = {};
        Nucleo::Compiler compiler;
        if (!compiler.Compile(ast, program)) {
            lastError = "Compile error: " + compiler.Error();
            std::cerr << "[NucleoScript] " << lastError << "\n";
            Logger::LogError("[Script] " + lastError);
            return false;
        }

        // 5. Bind native API
        Nucleo::NucleoAPI::Register(program,
            owner,
            owner ? owner->input : nullptr);

        // 6. Attach VM
        vm.SetProgram(&program);

        // 7. Resolve class name
        if (className.empty() && !program.classes.empty())
            className = program.classes[0].name;

        // 8. Instantiate + OnStart
        instance = vm.Instantiate(className);
        auto res  = vm.Call(instance, "OnStart");
        if (!res.ok) {
            lastError = "OnStart error: " + res.error;
            std::cerr << "[NucleoScript] " << lastError << "\n";
            Logger::LogError("[Script] " + lastError);
        }

        return lastError.empty();
    }

    // ── Component lifecycle ──────────────────────────────────────────────────
    void OnAttach() override
    {
        if (className.empty() && !scriptPath.empty())
            className = stemFromPath(scriptPath);
        if (!scriptPath.empty())
            Reload();
    }

    void Update(float dt) override
    {
        if (!instance) return;
        auto res = vm.Call(instance, "Update",
                           { Nucleo::Value::Float(dt) });
        if (!res.ok && lastError != res.error) {
            lastError = res.error;
            std::cerr << "[NucleoScript] Update error: " << lastError << "\n";
            Logger::LogError("[Script] Update: " + lastError);
        }
    }

    void OnDetach() override
    {
        if (instance)
            vm.Call(instance, "OnDestroy");
        instance.reset();
    }

    // ── Direct access for editor / debugging ────────────────────────────────
    Nucleo::VM&              GetVM()       { return vm; }
    Nucleo::ObjectRef        GetInstance() { return instance; }
    Nucleo::CompiledProgram& GetProgram()  { return program; }

private:
    Nucleo::CompiledProgram program;
    Nucleo::VM              vm;
    Nucleo::ObjectRef       instance;

    static std::string stemFromPath(const std::string& path)
    {
        namespace fs = std::filesystem;
        return fs::path(path).stem().string();
    }
};

} // namespace LightningEngine
