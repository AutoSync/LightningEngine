// EditorBridge — minimal runtime contract for the React/Tauri editor host.
//
// This bridge keeps a small, stable snapshot of editor runtime state that can
// be consumed by a desktop host or by future embedded tooling.
#pragma once

#include <string>

namespace LightningEngine::EditorBridge {

struct Status {
    bool running = false;
    float fps = 0.f;
    std::string project;
    std::string scene;
    std::string lastChange;
};

void SetProject(const std::string& projectName);
void SetScene(const std::string& scenePath);
void SetRunning(bool running);
void SetFps(float fps);
void SetLastChange(const std::string& message);

Status GetStatus();
std::string GetStatusJson();
void Reset();

} // namespace LightningEngine::EditorBridge
