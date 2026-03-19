// Node2D.h — 2D-specialised Node.
// Wraps Transform.Position.x/y and Transform.Scale.x/y behind a clean 2D API.
// Use Node2D for any entity that lives in screen/world space (sprites, UI anchors,
// physics bodies, etc.).
//
// CLASS HIERARCHY:
// Window -> GameInstance -> Level -> [ NODE2D : Node ] -> Component

#pragma once
#include "Node.h"
#include <cmath>

namespace LightningEngine {

class Node2D : public Node {
public:
    explicit Node2D(const char*        name = "Node2D") : Node(name) {}
    explicit Node2D(const std::string& name)            : Node(name) {}

    // -----------------------------------------------------------------------
    // Position
    // -----------------------------------------------------------------------

    void SetPosition(float x, float y)
    {
        transform.Position.x = x;
        transform.Position.y = y;
    }

    void SetPosition(Lightning::V2 v) { SetPosition(v.x, v.y); }

    Lightning::V2 GetPosition() const
    {
        return { transform.Position.x, transform.Position.y };
    }

    // Translate by delta each call.
    void Move(float dx, float dy)
    {
        transform.Position.x += dx;
        transform.Position.y += dy;
    }

    void Move(Lightning::V2 delta) { Move(delta.x, delta.y); }

    // -----------------------------------------------------------------------
    // Rotation (Z axis, degrees)
    // -----------------------------------------------------------------------

    void SetRotation(float degrees)
    {
        transform.Rotation.z = degrees;
    }

    float GetRotation() const { return transform.Rotation.z; }

    void Rotate(float degrees)
    {
        transform.Rotation.z += degrees;
    }

    // -----------------------------------------------------------------------
    // Scale
    // -----------------------------------------------------------------------

    void SetScale(float sx, float sy)
    {
        transform.Scale.x = sx;
        transform.Scale.y = sy;
    }

    void SetScale(float uniform) { SetScale(uniform, uniform); }

    Lightning::V2 GetScale() const
    {
        return { transform.Scale.x, transform.Scale.y };
    }

    // -----------------------------------------------------------------------
    // Helpers
    // -----------------------------------------------------------------------

    // Direction vector from this node toward a target world position.
    Lightning::V2 DirectionTo(Lightning::V2 target) const
    {
        Lightning::V2 wp = WorldPosition();
        float dx = target.x - wp.x;
        float dy = target.y - wp.y;
        float len = std::sqrtf(dx * dx + dy * dy);
        if (len < 0.0001f) return { 0.f, 0.f };
        return { dx / len, dy / len };
    }

    float DistanceTo(Lightning::V2 target) const
    {
        Lightning::V2 wp = WorldPosition();
        float dx = target.x - wp.x;
        float dy = target.y - wp.y;
        return std::sqrtf(dx * dx + dy * dy);
    }

    // Linearly interpolate position toward a target.
    void MoveTo(Lightning::V2 target, float speed, float dt)
    {
        Lightning::V2 dir = DirectionTo(target);
        float dist = DistanceTo(target);
        float step = speed * dt;
        if (step >= dist) {
            SetPosition(target);
        } else {
            Move(dir.x * step, dir.y * step);
        }
    }
};

} // namespace LightningEngine
