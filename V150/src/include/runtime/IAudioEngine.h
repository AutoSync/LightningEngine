// IAudioEngine.h — Pure interface for the Lightning Engine audio subsystem.
//
// Audio is not yet implemented in the runtime; this interface establishes
// the contract for when it is (or for mock/plugin implementations).
//
// See: docs/developers/01-architecture.md (Runtime layer).
// See: docs/developers/06-restructure-roadmap.md (Fase 2 for concrete impl).
#pragma once

#include <cstdint>
#include <string>

namespace LightningEngine {

// Opaque handle to a loaded audio clip. 0 = invalid.
using AudioClipHandle = uint32_t;
// Opaque handle to a playing audio instance. 0 = invalid.
using AudioInstanceHandle = uint32_t;

class IAudioEngine {
public:
    virtual ~IAudioEngine() = default;

    // ── Lifecycle ────────────────────────────────────────────────────────
    // Called once per frame for streaming, fade, and cleanup.
    virtual void Update(float dt) = 0;

    // ── Asset management ─────────────────────────────────────────────────
    // Load a clip from file and return a handle. Returns 0 on failure.
    virtual AudioClipHandle LoadClip(const std::string& path)        = 0;
    // Release a previously loaded clip.
    virtual void            UnloadClip(AudioClipHandle handle)       = 0;

    // ── Playback ─────────────────────────────────────────────────────────
    // Play a clip. volume [0,1], pitch multiplier [0.5, 2]. Returns instance.
    virtual AudioInstanceHandle Play(AudioClipHandle clip,
                                     float volume = 1.f,
                                     float pitch  = 1.f,
                                     bool  loop   = false) = 0;

    virtual void Pause (AudioInstanceHandle instance) = 0;
    virtual void Resume(AudioInstanceHandle instance) = 0;
    virtual void Stop  (AudioInstanceHandle instance) = 0;

    virtual bool IsPlaying(AudioInstanceHandle instance) const = 0;

    // ── 2D positional audio ──────────────────────────────────────────────
    virtual void SetListenerPosition(float x, float y)                          = 0;
    virtual void SetInstancePosition(AudioInstanceHandle instance,
                                     float x, float y)                          = 0;

    // ── Global volume ────────────────────────────────────────────────────
    virtual void SetMasterVolume(float volume) = 0;  // [0, 1]
    virtual float GetMasterVolume() const      = 0;
};

} // namespace LightningEngine
