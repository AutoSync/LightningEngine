#pragma once

#include "../include/Renderer.h"
#include <SDL3/SDL.h>
#include <filesystem>
#include <iostream>
#include <string>

namespace LightningTests {

class TestLog {
public:
    int passed = 0;
    int failed = 0;

    void Expect(bool condition, const char* name, const std::string& detail = "")
    {
        if (condition) {
            passed++;
            std::cout << "[PASS] " << name;
            if (!detail.empty()) std::cout << " :: " << detail;
            std::cout << "\n";
            return;
        }

        failed++;
        std::cout << "[FAIL] " << name;
        if (!detail.empty()) std::cout << " :: " << detail;
        std::cout << "\n";
    }

    int ExitCode() const { return failed == 0 ? 0 : 1; }
};

inline bool FileExists(const char* path)
{
    return path && std::filesystem::exists(std::filesystem::path(path));
}

inline int RunImagePipelineTests()
{
    using namespace LightningEngine;

    TestLog t;

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        t.Expect(false, "SDL_Init", SDL_GetError());
        std::cout << "[SUMMARY] passed=" << t.passed << " failed=" << t.failed << "\n";
        return t.ExitCode();
    }

    SDL_Window* window = SDL_CreateWindow("LightningEngine Tests", 640, 360, SDL_WINDOW_HIDDEN);
    if (!window) {
        t.Expect(false, "SDL_CreateWindow", SDL_GetError());
        SDL_Quit();
        std::cout << "[SUMMARY] passed=" << t.passed << " failed=" << t.failed << "\n";
        return t.ExitCode();
    }

    SDL_GPUDevice* device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, nullptr);
    if (!device) {
        t.Expect(false, "SDL_CreateGPUDevice", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        std::cout << "[SUMMARY] passed=" << t.passed << " failed=" << t.failed << "\n";
        return t.ExitCode();
    }

    if (!SDL_ClaimWindowForGPUDevice(device, window)) {
        t.Expect(false, "SDL_ClaimWindowForGPUDevice", SDL_GetError());
        SDL_DestroyGPUDevice(device);
        SDL_DestroyWindow(window);
        SDL_Quit();
        std::cout << "[SUMMARY] passed=" << t.passed << " failed=" << t.failed << "\n";
        return t.ExitCode();
    }

    Renderer renderer(device, window);

    const char* splashPath = "assets/splash/splashscreen.png";
    const char* logoPath = "assets/icons/lightning.png";
    const char* missingPath = "assets/icons/does_not_exist.png";

    t.Expect(FileExists(splashPath), "Asset splashscreen existe", splashPath);
    t.Expect(FileExists(logoPath), "Asset logo existe", logoPath);

    Texture splash = renderer.LoadTexture(splashPath);
    t.Expect(splash.IsValid(), "LoadTexture carrega PNG splash", splashPath);
    t.Expect(splash.GetWidth() > 0 && splash.GetHeight() > 0,
             "Dimensoes validas da splash",
             std::to_string(splash.GetWidth()) + "x" + std::to_string(splash.GetHeight()));

    Texture logo = renderer.LoadTexture(logoPath);
    t.Expect(logo.IsValid(), "LoadTexture carrega PNG logo", logoPath);

    Texture missing = renderer.LoadTexture(missingPath);
    t.Expect(!missing.IsValid(), "LoadTexture falha com arquivo ausente", missingPath);

    renderer.Clear();
    renderer.SetDrawColor(255, 255, 255, 255);
    renderer.DrawTexture(splash, 10.f, 10.f, 128.f, 128.f);
    renderer.DrawTexture(logo, 150.f, 10.f, 64.f, 64.f);
    renderer.Present();
    t.Expect(true, "Pipeline 2D desenha e apresenta sem crash");

    splash.Release();
    logo.Release();
    missing.Release();

    renderer.Release();

    SDL_ReleaseWindowFromGPUDevice(device, window);
    SDL_DestroyGPUDevice(device);
    SDL_DestroyWindow(window);
    SDL_Quit();

    std::cout << "[SUMMARY] passed=" << t.passed << " failed=" << t.failed << "\n";
    return t.ExitCode();
}

} // namespace LightningTests
