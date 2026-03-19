// GamePreviewWindow.h — Standalone game preview in a separate SDL window.
//
// Opens a second SDL window sharing the same GPU device as the editor.
// Loads and runs the project's current scene independently from the editor.
//
// Usage (in EditorApp):
//   GamePreviewWindow preview;
//
//   // On Play button press:
//   preview.Open(renderer.GetDevice(), pm.AbsScene(currentScene),
//                1280, 720, "Game Preview");
//
//   // In Update() each frame:
//   if (preview.IsOpen())
//       preview.Tick(dt);
//
//   // On Stop button press:
//   preview.Close();
//
// The preview window handles its own SDL event loop for input, and renders
// the game scene independently. Closing the preview window from the OS also
// calls Close() automatically.
#pragma once
#include <string>
#include <memory>
#include <vector>
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include "Renderer.h"
#include "InputManager.h"
#include "Level.h"
#include "ProjectManager.h"

namespace LightningEngine {

class GamePreviewWindow {
public:
    GamePreviewWindow()  = default;
    ~GamePreviewWindow() { Close(); }

    // Non-copyable.
    GamePreviewWindow(const GamePreviewWindow&)            = delete;
    GamePreviewWindow& operator=(const GamePreviewWindow&) = delete;

    // ── Open ─────────────────────────────────────────────────────────────────
    // device      — SDL_GPUDevice* shared with the editor (from Renderer::GetDevice()).
    // scenePath   — absolute path to the .lescene file to load.
    // w/h         — initial window size in pixels.
    // title       — OS window title string.
    bool Open(SDL_GPUDevice* device,
              const std::string& projectPath,
              const std::string& scenePath,
              int w = 1280, int h = 720,
              const char* title = "Game Preview")
    {
        if (sdlWin) return true; // already open

        this->device = device;

        sdlWin = SDL_CreateWindow(title, w, h, SDL_WINDOW_RESIZABLE);
        if (!sdlWin) {
            std::cerr << "[GamePreview] SDL_CreateWindow failed: "
                      << SDL_GetError() << "\n";
            return false;
        }

        // Claim the new window for the shared GPU device.
        if (!SDL_ClaimWindowForGPUDevice(device, sdlWin)) {
            std::cerr << "[GamePreview] SDL_ClaimWindowForGPUDevice failed: "
                      << SDL_GetError() << "\n";
            SDL_DestroyWindow(sdlWin);
            sdlWin = nullptr;
            return false;
        }

        // Create a Renderer that targets the preview window (same device).
        previewRenderer = std::make_unique<Renderer>(device, sdlWin);

        // Open the project and load the scene.
        if (!pm.Open(projectPath)) {
            std::cerr << "[GamePreview] Cannot open project at: " << projectPath << "\n";
            Close();
            return false;
        }

        previewLevel = std::make_unique<PreviewLevel>();
        previewLevel->SetContext(*previewRenderer, previewInput, nullptr);

        if (!pm.LoadScene(scenePath, *previewLevel)) {
            std::cerr << "[GamePreview] Cannot load scene: " << scenePath << "\n";
            Close();
            return false;
        }

        previewLevel->Initialize();
        winID = SDL_GetWindowID(sdlWin);
        return true;
    }

    // ── Close ─────────────────────────────────────────────────────────────────
    void Close()
    {
        if (previewLevel) { previewLevel->Shutdown(); previewLevel.reset(); }
        previewRenderer.reset();

        if (sdlWin && device) {
            SDL_ReleaseWindowFromGPUDevice(device, sdlWin);
            SDL_DestroyWindow(sdlWin);
        }
        sdlWin  = nullptr;
        device  = nullptr;
        winID   = 0;
    }

    bool IsOpen() const { return sdlWin != nullptr; }

    // ── Tick ──────────────────────────────────────────────────────────────────
    // Call once per frame from the editor's Update loop.
    // dt — delta time in milliseconds (same convention as GameInstance).
    void Tick(float dt)
    {
        if (!sdlWin || !previewRenderer || !previewLevel) return;

        // Process pending SDL events for this window.
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
                ev.window.windowID == winID) {
                Close();
                return;
            }
            if (ev.window.windowID == winID)
                previewInput.ProcessEvent(ev);
        }
        previewInput.Update();

        // Update and render the game scene.
        previewLevel->Update(dt);

        previewRenderer->Clear();
        previewLevel->Render();
        previewRenderer->Present();
    }

    SDL_Window* GetWindow() const { return sdlWin; }

private:
    // Minimal Level subclass for the preview.
    class PreviewLevel : public Level {
    public:
        void Initialize() override
        {
            // Call OnStart on all scripts.
            for (auto& n : GetNodes())
                if (n) n->Update(0.f); // warm-up tick
        }
        void Shutdown() override {}

        // Expose SetContext so GamePreviewWindow can inject renderer/input.
        void SetContext(Renderer& r, InputManager& im, Level* /*parent*/)
        {
            for (auto& n : GetNodes())
                if (n) n->SetContext(&r, &im, this);
        }
    };

    SDL_Window*             sdlWin          = nullptr;
    SDL_GPUDevice*          device          = nullptr;
    Uint32                  winID           = 0;
    std::unique_ptr<Renderer>      previewRenderer;
    InputManager                   previewInput;
    std::unique_ptr<PreviewLevel>  previewLevel;
    ProjectManager                 pm;
};

} // namespace LightningEngine
