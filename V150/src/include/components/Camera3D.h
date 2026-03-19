// Camera3D.h — 3D perspective camera component.
// Attach to any Node; each frame the component computes view and projection
// matrices from the node's world transform and pushes them to the Renderer.
//
// Usage:
//   auto* cam = cameraNode->AddComponent<Camera3DComponent>();
//   cam->fovDeg   = 60.f;
//   cam->nearClip = 0.1f;
//   cam->farClip  = 1000.f;
//
// The component respects the node's Position (eye position) and Rotation
// (Euler angles in degrees: pitch=X, yaw=Y, roll=Z).  It calls
// renderer.SetView() and renderer.SetProjection() on every Update().
//
// Optionally set a Light3D via sceneLight — it is pushed to the renderer
// every frame so a single camera drives the whole 3D scene.
#pragma once
#include "../Component.h"
#include "../Renderer.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace LightningEngine {

class Camera3DComponent : public Component {
public:
    float    fovDeg      = 60.f;
    float    nearClip    = 0.1f;
    float    farClip     = 1000.f;
    bool     applyLight  = true;   // push sceneLight to renderer each frame
    Light3D  sceneLight;

    void Update(float /*dt*/) override
    {
        if (!owner || !owner->renderer) return;

        Renderer& r = *owner->renderer;

        // ── View matrix ──────────────────────────────────────────────────────
        // Build from the node's world position and Euler rotation (degrees).
        glm::vec3 pos  = glm::vec3(owner->transform.Position.x,
                                   owner->transform.Position.y,
                                   owner->transform.Position.z);

        float yaw   = glm::radians(owner->transform.Rotation.y);
        float pitch = glm::radians(owner->transform.Rotation.x);

        // Forward vector from yaw and pitch
        glm::vec3 forward;
        forward.x = std::cos(pitch) * std::sin(yaw);
        forward.y = std::sin(pitch);
        forward.z = std::cos(pitch) * std::cos(yaw);
        forward   = glm::normalize(forward);

        glm::mat4 view = glm::lookAt(pos, pos + forward, glm::vec3(0.f, 1.f, 0.f));
        r.SetView(view);

        // ── Projection matrix ────────────────────────────────────────────────
        int sw = 1280, sh = 720;
        SDL_GetWindowSize(r.GetWindow(), &sw, &sh);
        float aspect = (sh > 0) ? (float)sw / (float)sh : 16.f / 9.f;

        glm::mat4 proj = glm::perspective(glm::radians(fovDeg), aspect, nearClip, farClip);
        // Vulkan clip-space: Y is flipped relative to OpenGL
        proj[1][1] *= -1.f;
        r.SetProjection(proj);

        // ── Light ────────────────────────────────────────────────────────────
        if (applyLight)
            r.SetLight(sceneLight);
    }
};

} // namespace LightningEngine
