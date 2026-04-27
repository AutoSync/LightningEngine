// IPhysicsWorld.h — Pure interface for the Lightning Engine physics subsystem.
//
// Abstracts kinematic integration and AABB collision from the concrete
// implementation (MotionPhysics / RigidBody2D in Physics.h).
//
// See: src/include/Physics.h (concrete 2D physics).
// See: docs/developers/01-architecture.md
#pragma once

#include "Math.h"

namespace LightningEngine {

struct Rect2D;
struct RigidBody2D;

// Collision result returned by QueryOverlap / Raycast.
struct HitResult2D {
    bool hit        = false;
    uint32_t entity = 0;     // Entity ID hit (0 = none)
    Lightning::V2 point;     // World-space contact point
    Lightning::V2 normal;    // Surface normal at contact
    float distance  = 0.f;  // Distance from origin (raycast)
};

class IPhysicsWorld {
public:
    virtual ~IPhysicsWorld() = default;

    // ── Lifecycle ────────────────────────────────────────────────────────
    // Step the simulation by dt seconds.
    virtual void Step(float dt) = 0;

    // ── Body registration ────────────────────────────────────────────────
    // Associate a RigidBody2D with an entity ID.
    // The IPhysicsWorld does NOT own the body — caller manages lifetime.
    virtual void RegisterBody  (uint32_t entityId, RigidBody2D* body) = 0;
    virtual void UnregisterBody(uint32_t entityId)                    = 0;

    // ── Collision queries ────────────────────────────────────────────────
    // Broad-phase AABB overlap check.
    virtual bool QueryOverlap(const Rect2D& region,
                              uint32_t* outEntities, int maxResults) const = 0;

    // Simple 2D raycast. Returns first hit along direction.
    virtual HitResult2D Raycast(Lightning::V2 origin,
                                Lightning::V2 direction,
                                float maxDistance) const = 0;

    // ── World settings ───────────────────────────────────────────────────
    virtual void SetGravity(Lightning::V2 gravity) = 0;
    virtual Lightning::V2 GetGravity() const       = 0;
};

} // namespace LightningEngine
