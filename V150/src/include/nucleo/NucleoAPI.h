// NucleoAPI.h — Registers C++ engine functions as Nucleo native callables.
// Call NucleoAPI::Register(program, node, renderer, input) once per script
// instance before executing. Native functions can access the bound node.
//
// Available script-side (examples):
//   Log(msg)                     — print to stdout
//   Input.IsKeyDown(scancode)    — keyboard query
//   Input.IsMousePressed(btn)    — mouse button
//   Input.GetMouseX/Y()          — mouse position
//   Node.Move(dx, dy, dz)        — translate the bound node
//   Node.SetPosition(x, y, z)
//   Node.GetPositionX/Y/Z()
//   Node.SetActive(bool)
//   Math.Abs/Min/Max/Lerp/Clamp/Sqrt/Sin/Cos
#pragma once
#include "NucleoBytecode.h"
#include "../InputManager.h"
#include "../Node.h"
#include "../Logger.h"
#include <cmath>
#include <iostream>

namespace Nucleo {

class NucleoAPI {
public:
    // Register all built-in native functions into the CompiledProgram.
    // `node` and `im` may be nullptr; functions will return Null if so.
    static void Register(CompiledProgram& prog,
                         LightningEngine::Node*         node,
                         LightningEngine::InputManager* im)
    {
        // ── Logging ──────────────────────────────────────────────────────────
        addNative(prog, "Log", [](const std::vector<Value>& a) -> Value {
            std::string msg;
            for (const auto& v : a) msg += v.toString();
            std::cout << msg << "\n";
            Logger::LogScript(msg);
            return Value::Null();
        });

        // ── Math ─────────────────────────────────────────────────────────────
        addNative(prog, "Math.Abs",   [](const std::vector<Value>& a) { return Value::Float(std::abs(a[0].toFloat())); });
        addNative(prog, "Math.Sqrt",  [](const std::vector<Value>& a) { return Value::Float(std::sqrt(a[0].toFloat())); });
        addNative(prog, "Math.Sin",   [](const std::vector<Value>& a) { return Value::Float(std::sin(a[0].toFloat())); });
        addNative(prog, "Math.Cos",   [](const std::vector<Value>& a) { return Value::Float(std::cos(a[0].toFloat())); });
        addNative(prog, "Math.Floor", [](const std::vector<Value>& a) { return Value::Float(std::floor(a[0].toFloat())); });
        addNative(prog, "Math.Ceil",  [](const std::vector<Value>& a) { return Value::Float(std::ceil(a[0].toFloat())); });
        addNative(prog, "Math.Min",   [](const std::vector<Value>& a) { return Value::Float(std::min(a[0].toFloat(), a[1].toFloat())); });
        addNative(prog, "Math.Max",   [](const std::vector<Value>& a) { return Value::Float(std::max(a[0].toFloat(), a[1].toFloat())); });
        addNative(prog, "Math.Clamp", [](const std::vector<Value>& a) { return Value::Float(std::max(a[1].toFloat(), std::min(a[2].toFloat(), a[0].toFloat()))); });
        addNative(prog, "Math.Lerp",  [](const std::vector<Value>& a) {
            float t = a[2].toFloat();
            return Value::Float(a[0].toFloat() + t * (a[1].toFloat() - a[0].toFloat()));
        });
        addNative(prog, "Math.PI",    [](const std::vector<Value>&) { return Value::Float(3.14159265f); });

        // ── Input ────────────────────────────────────────────────────────────
        addNative(prog, "Input.IsKeyDown", [im](const std::vector<Value>& a) -> Value {
            if (!im || a.empty()) return Value::Bool(false);
            return Value::Bool(im->IsKeyDown((SDL_Scancode)a[0].toInt()));
        });
        addNative(prog, "Input.IsKeyPressed", [im](const std::vector<Value>& a) -> Value {
            if (!im || a.empty()) return Value::Bool(false);
            return Value::Bool(im->IsKeyPressed((SDL_Scancode)a[0].toInt()));
        });
        addNative(prog, "Input.IsKeyReleased", [im](const std::vector<Value>& a) -> Value {
            if (!im || a.empty()) return Value::Bool(false);
            return Value::Bool(im->IsKeyReleased((SDL_Scancode)a[0].toInt()));
        });
        addNative(prog, "Input.IsMousePressed", [im](const std::vector<Value>& a) -> Value {
            if (!im || a.empty()) return Value::Bool(false);
            return Value::Bool(im->IsMousePressed(a[0].toInt()));
        });
        addNative(prog, "Input.IsMouseDown", [im](const std::vector<Value>& a) -> Value {
            if (!im || a.empty()) return Value::Bool(false);
            return Value::Bool(im->IsMouseDown(a[0].toInt()));
        });
        addNative(prog, "Input.GetMouseX", [im](const std::vector<Value>&) -> Value {
            return Value::Float(im ? im->GetMouseX() : 0.f);
        });
        addNative(prog, "Input.GetMouseY", [im](const std::vector<Value>&) -> Value {
            return Value::Float(im ? im->GetMouseY() : 0.f);
        });

        // ── Node / Transform ─────────────────────────────────────────────────
        addNative(prog, "Node.SetPosition", [node](const std::vector<Value>& a) -> Value {
            if (node && a.size() >= 3) {
                node->transform.Position = Lightning::V3(a[0].toFloat(), a[1].toFloat(), a[2].toFloat());
            }
            return Value::Null();
        });
        addNative(prog, "Node.Move", [node](const std::vector<Value>& a) -> Value {
            if (node && a.size() >= 2) {
                node->transform.Position.x += a[0].toFloat();
                node->transform.Position.y += a[1].toFloat();
                if (a.size() >= 3) node->transform.Position.z += a[2].toFloat();
            }
            return Value::Null();
        });
        addNative(prog, "Node.GetPositionX", [node](const std::vector<Value>&) -> Value {
            return Value::Float(node ? node->transform.Position.x : 0.f);
        });
        addNative(prog, "Node.GetPositionY", [node](const std::vector<Value>&) -> Value {
            return Value::Float(node ? node->transform.Position.y : 0.f);
        });
        addNative(prog, "Node.GetPositionZ", [node](const std::vector<Value>&) -> Value {
            return Value::Float(node ? node->transform.Position.z : 0.f);
        });
        addNative(prog, "Node.SetRotation", [node](const std::vector<Value>& a) -> Value {
            if (node && a.size() >= 3) {
                node->transform.Rotation = Lightning::V3(a[0].toFloat(), a[1].toFloat(), a[2].toFloat());
            }
            return Value::Null();
        });
        addNative(prog, "Node.SetScale", [node](const std::vector<Value>& a) -> Value {
            if (node && a.size() >= 3) {
                node->transform.Scale = Lightning::V3(a[0].toFloat(), a[1].toFloat(), a[2].toFloat());
            }
            return Value::Null();
        });
        addNative(prog, "Node.SetActive", [node](const std::vector<Value>& a) -> Value {
            if (node && !a.empty()) node->active = a[0].toBool();
            return Value::Null();
        });
        addNative(prog, "Node.GetName", [node](const std::vector<Value>&) -> Value {
            return Value::String(node ? node->name : "");
        });

        // ── Scancode constants ────────────────────────────────────────────────
        // Expose common keys as integers so scripts can write Key.W, Key.Space etc.
        auto kc = [&](const std::string& n, int sc) {
            addNative(prog, "Key." + n, [sc](const std::vector<Value>&) {
                return Value::Int(sc);
            });
        };
        kc("W", SDL_SCANCODE_W);  kc("A", SDL_SCANCODE_A);
        kc("S", SDL_SCANCODE_S);  kc("D", SDL_SCANCODE_D);
        kc("Space", SDL_SCANCODE_SPACE);
        kc("Left",  SDL_SCANCODE_LEFT);  kc("Right", SDL_SCANCODE_RIGHT);
        kc("Up",    SDL_SCANCODE_UP);    kc("Down",  SDL_SCANCODE_DOWN);
        kc("Escape",SDL_SCANCODE_ESCAPE);
        kc("Return",SDL_SCANCODE_RETURN);
        kc("LShift",SDL_SCANCODE_LSHIFT);
    }

private:
    static void addNative(CompiledProgram& prog,
                          const std::string& name, NativeFn fn)
    {
        // Deduplicate
        for (auto& n : prog.natives)
            if (n.name == name) { n.fn = std::move(fn); return; }
        prog.natives.push_back({ name, std::move(fn) });
    }
};

} // namespace Nucleo
