// IInputManager.h — Pure interface for the Lightning Engine input system.
//
// Plugins and gameplay code should hold IInputManager*, not InputManager*.
// This decouples from SDL3 and allows headless/mock input in tests.
//
// See: src/include/InputManager.h (concrete SDL3 implementation).
#pragma once

#include <cstdint>
#include <string>

// Forward-declare SDL scancode type so consumers don't need SDL3 headers.
// Cast to SDL_Scancode before passing to the concrete implementation.
using LEKeyScancode = int;

namespace LightningEngine {

class IInputManager {
public:
    virtual ~IInputManager() = default;

    // ── Lifecycle ────────────────────────────────────────────────────────
    // Call once per frame BEFORE polling events.
    virtual void Update() = 0;

    // ── Keyboard ─────────────────────────────────────────────────────────
    virtual bool IsKeyDown    (LEKeyScancode key) const = 0;  // held this frame
    virtual bool IsKeyPressed (LEKeyScancode key) const = 0;  // went down this frame
    virtual bool IsKeyReleased(LEKeyScancode key) const = 0;  // went up this frame

    // ── Mouse ────────────────────────────────────────────────────────────
    // button: 1=left  2=middle  3=right
    virtual bool  IsMouseDown    (int button) const = 0;
    virtual bool  IsMousePressed (int button) const = 0;
    virtual bool  IsMouseReleased(int button) const = 0;
    virtual float GetMouseX()      const = 0;
    virtual float GetMouseY()      const = 0;
    virtual float GetScrollWheelY() const = 0;

    // ── Text input ───────────────────────────────────────────────────────
    virtual const std::string& GetTextInput() const = 0;
    virtual bool HasDeleteBack() const = 0;

    // ── Quit ─────────────────────────────────────────────────────────────
    virtual bool ShouldQuit() const = 0;

    // ── Gamepad ──────────────────────────────────────────────────────────
    virtual bool  HasGamepad() const = 0;
    // btn: SDL_GamepadButton int values
    virtual bool  IsGamepadButtonDown    (int btn) const = 0;
    virtual bool  IsGamepadButtonPressed (int btn) const = 0;
    virtual bool  IsGamepadButtonReleased(int btn) const = 0;
    virtual float GetGamepadAxis(int axis) const = 0;
};

} // namespace LightningEngine
