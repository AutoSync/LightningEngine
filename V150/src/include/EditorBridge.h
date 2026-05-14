// EditorBridge — runtime contract for the native editor host.
//
// Two layers:
//   1. Status snapshot (running / fps / project / scene / lastChange).
//   2. Scene contract: live serialization of the active Level + command
//      dispatch (select / move / create / delete) wired through editor-side
//      handlers. The bridge owns no policy — it forwards commands to the
//      handlers registered by EditorApp so undo, hierarchy refresh and the
//      Renderer pipeline remain the source of truth.
//
// The scene layer is opt-in: until SetActiveLevel() and command handlers are
// registered, GetSceneJson() returns an empty scene and DispatchCommand()
// reports failure. This lets the front-end always poll without crashing the
// engine.
#pragma once

#include <functional>
#include <string>

namespace LightningEngine {
    class Level; // fwd
}

namespace LightningEngine::EditorBridge {

// ── Status (existing) ─────────────────────────────────────────────────────
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
void SaveStatusSnapshot(const std::string& filePath = "editor-bridge-status.json");

Status GetStatus();
std::string GetStatusJson();
void Reset();

// ── Scene contract ────────────────────────────────────────────────────────
//
// Wire the active level so the bridge can serialize it on demand.
// Pass nullptr to detach (e.g. on splash / project close).
void SetActiveLevel(LightningEngine::Level* level);

// Returns a JSON snapshot of the active level. Shape:
// {
//   "scene": "scenes/main.lescene",
//   "selectedId": "0x...." | null,
//   "nodes": [
//     { "id":"0x..", "parentId":"0x.." | null, "name":"Player", "tag":"...",
//       "active":true, "components":["SpriteRenderer", ...],
//       "transform":{ "px":0,"py":0,"pz":0, "rx":0,"ry":0,"rz":0,
//                     "sx":1,"sy":1,"sz":1 } }
//   ]
// }
std::string GetSceneJson();

// Persists the current scene snapshot to disk (companion to
// SaveStatusSnapshot). The desktop editor can poll this file to mirror the
// live scene while a direct bridge is not in place.
void SaveSceneSnapshot(const std::string& filePath = "editor-bridge-scene.json");

// Drains the command queue file written by the editor host (one JSON object
// per line, JSONL). For each command, DispatchCommand is invoked and the
// queue file is truncated. Safe to call every editor frame; does nothing if
// the file is missing or empty. Returns the number of commands processed.
int DrainCommandQueue(const std::string& filePath = "editor-bridge-commands.jsonl");

// Marks which node should be highlighted in subsequent snapshots.
// The pointer must remain valid (owned by the active Level). Pass nullptr
// to clear. The bridge does NOT own this pointer — the editor must call
// SetSelectedNodePtr(nullptr) before destroying the node.
void SetSelectedNodePtr(const void* nodePtr);

// ── Command dispatch ──────────────────────────────────────────────────────
//
// Editor registers concrete handlers; the front-end (Tauri host) feeds JSON
// commands and the bridge routes them. Handlers run on the editor thread
// (engine update tick).
//
// Command JSON shapes accepted by DispatchCommand:
//   { "op":"select",      "id":"0x..."          }
//   { "op":"deselect"                            }
//   { "op":"setTransform","id":"0x...",
//     "px":0,"py":0,"pz":0 }                    // any axis optional
//   { "op":"createNode",  "parentId":"0x..."|null, "name":"Node",
//                         "archetype":"empty" }
//   { "op":"deleteNode",  "id":"0x..."          }
//
// Returns a JSON object: { "ok": bool, "error": "..." (when !ok) }.
struct CommandHandlers {
    // All handlers are optional. Missing handlers cause DispatchCommand to
    // return { ok:false, error:"unsupported" } for that op.
    std::function<bool(const std::string& nodeId)>                       selectById;
    std::function<bool()>                                                deselect;
    std::function<bool(const std::string& nodeId,
                       bool hasX, float x,
                       bool hasY, float y,
                       bool hasZ, float z)>                              setTransform;
    std::function<std::string(const std::string& parentId,
                              const std::string& name,
                              const std::string& archetype)>             createNode;
    std::function<bool(const std::string& nodeId)>                       deleteNode;
};

void RegisterCommandHandlers(CommandHandlers handlers);
void ClearCommandHandlers();

// Parses the JSON command and routes to the matching handler.
std::string DispatchCommand(const std::string& commandJson);

} // namespace LightningEngine::EditorBridge
