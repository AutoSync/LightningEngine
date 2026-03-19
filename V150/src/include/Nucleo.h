// Nucleo.h — Umbrella header for the Nucleo scripting subsystem.
//
// Include this single header to get the full scripting pipeline:
//   Lexer → Parser → Compiler → VM → NativeAPI → ScriptComponent
//
// For component-only usage (most common):
//   #include "Nucleo.h"
//   node->AddComponent<LightningEngine::NucleoScriptComponent>()
//      ->SetScript("scripts/MyClass.spark");
//
// For headless compilation / tooling:
//   Nucleo::Lexer, Nucleo::Parser, Nucleo::Compiler, Nucleo::VM
#pragma once

// Core pipeline
#include "nucleo/NucleoToken.h"
#include "nucleo/NucleoLexer.h"
#include "nucleo/NucleoAST.h"
#include "nucleo/NucleoParser.h"
#include "nucleo/NucleoValue.h"
#include "nucleo/NucleoBytecode.h"
#include "nucleo/NucleoCompiler.h"
#include "nucleo/NucleoVM.h"

// Engine-side bindings
#include "nucleo/NucleoAPI.h"

// Scene component
#include "nucleo/NucleoScript.h"
