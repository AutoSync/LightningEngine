// IRenderer.h — Pure interface for the Lightning Engine renderer.
//
// Goals:
//   - Decouple Core and plugins from the SDL3/GPU concrete implementation.
//   - Allow mocking in tests and headless builds.
//   - Any subsystem/plugin that draws should hold a pointer to IRenderer,
//     not to the concrete Renderer class.
//
// See: src/include/Renderer.h (concrete implementation).
// See: docs/developers/01-architecture.md (Runtime layer rules).
#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include "Texture.h"
#include "Framebuffer.h"
#include "Mesh.h"

namespace LightningEngine {

struct Light3D;  // forward-declared from Renderer.h

class IRenderer {
public:
    virtual ~IRenderer() = default;

    // ── Frame lifecycle ──────────────────────────────────────────────────
    virtual void SetClearColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) = 0;
    virtual void Clear()   = 0;
    virtual void Present() = 0;

    // ── Texture loading ──────────────────────────────────────────────────
    virtual Texture LoadTexture(const char* path)                                         = 0;
    virtual Texture LoadTextureFromPixels(const void* rgba8, int w, int h,
                                          bool nearestFilter = false)                     = 0;

    // ── Framebuffer ──────────────────────────────────────────────────────
    virtual void BeginRenderToTexture(Framebuffer& fb) = 0;
    virtual void EndRenderToTexture()                  = 0;
    virtual void BlurTexture(Framebuffer& dest, Texture& src, float radius = 4.f) = 0;

    // ── Camera ───────────────────────────────────────────────────────────
    virtual void  SetCameraOffset(float x, float y) = 0;
    virtual void  SetCameraZoom(float zoom)          = 0;
    virtual void  SetCameraOrigin(float x, float y)  = 0;
    virtual float GetCameraX()      const = 0;
    virtual float GetCameraY()      const = 0;
    virtual float GetCameraZoom()   const = 0;

    virtual void BeginScreenSpace() = 0;
    virtual void EndScreenSpace()   = 0;
    virtual void PushDrawState()    = 0;
    virtual void PopDrawState()     = 0;

    // ── 2D draw ──────────────────────────────────────────────────────────
    virtual void SetDrawColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) = 0;
    virtual void DrawRect(float x, float y, float w, float h)                   = 0;
    virtual void FillRect(float x, float y, float w, float h)                   = 0;
    virtual void DrawLine(float x1, float y1, float x2, float y2,
                          float thickness = 1.f)                                 = 0;
    virtual void DrawCircle(float cx, float cy, float radius, int segs = 0)     = 0;
    virtual void FillCircle(float cx, float cy, float radius, int segs = 0)     = 0;

    virtual void DrawTexture(Texture& tex, float x, float y,
                             float w = 0.f, float h = 0.f)                      = 0;
    virtual void DrawTextureRegion(Texture& tex, float x, float y,
                                   float w, float h,
                                   float u0, float v0, float u1, float v1)       = 0;
    virtual void DrawTextureEx(Texture& tex, float x, float y, float w, float h,
                               float angle, float pivX, float pivY)              = 0;

    virtual void SetScissor(float x, float y, float w, float h) = 0;
    virtual void ClearScissor()                                  = 0;

    // ── 3D API ───────────────────────────────────────────────────────────
    virtual void UploadMesh(Mesh& mesh)                                              = 0;
    virtual void ReleaseMesh(Mesh& mesh)                                             = 0;
    virtual void DrawMesh(Mesh& mesh, const glm::mat4& model,
                          Texture* albedo = nullptr)                                 = 0;
    virtual void SetView(const glm::mat4& v)       = 0;
    virtual void SetProjection(const glm::mat4& p) = 0;
    virtual void SetLight(const Light3D& l)        = 0;

    virtual const glm::mat4& GetView()       const = 0;
    virtual const glm::mat4& GetProjection() const = 0;
};

} // namespace LightningEngine
