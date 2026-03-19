// Node3D.h — 3D-specialised Node.
// Wraps Transform behind a clean 3D API.
// Use Node3D for any entity in 3D world space.
//
// CLASS HIERARCHY:
// Window -> GameInstance -> Level -> [ NODE3D : Node ] -> Component
#pragma once
#include "Node.h"
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace LightningEngine {

class Node3D : public Node {
public:
    explicit Node3D(const char*        name = "Node3D") : Node(name) {}
    explicit Node3D(const std::string& name)             : Node(name) {}

    // -----------------------------------------------------------------------
    // Position
    // -----------------------------------------------------------------------
    void SetPosition(float x, float y, float z)
    {
        transform.Position.x = x;
        transform.Position.y = y;
        transform.Position.z = z;
    }

    void SetPosition(Lightning::V3 v) { SetPosition(v.x, v.y, v.z); }

    Lightning::V3 GetPosition() const
    {
        return { transform.Position.x, transform.Position.y, transform.Position.z };
    }

    void Move(float dx, float dy, float dz)
    {
        transform.Position.x += dx;
        transform.Position.y += dy;
        transform.Position.z += dz;
    }

    void Move(Lightning::V3 delta) { Move(delta.x, delta.y, delta.z); }

    // -----------------------------------------------------------------------
    // Rotation (Euler angles, degrees — applied in YXZ order)
    // -----------------------------------------------------------------------
    void SetRotation(float pitch, float yaw, float roll)
    {
        transform.Rotation.x = pitch;
        transform.Rotation.y = yaw;
        transform.Rotation.z = roll;
    }

    void SetRotation(Lightning::V3 euler) { SetRotation(euler.x, euler.y, euler.z); }

    Lightning::V3 GetRotation() const
    {
        return { transform.Rotation.x, transform.Rotation.y, transform.Rotation.z };
    }

    void Rotate(float dpitch, float dyaw, float droll)
    {
        transform.Rotation.x += dpitch;
        transform.Rotation.y += dyaw;
        transform.Rotation.z += droll;
    }

    // -----------------------------------------------------------------------
    // Scale
    // -----------------------------------------------------------------------
    void SetScale(float x, float y, float z)
    {
        transform.Scale.x = x;
        transform.Scale.y = y;
        transform.Scale.z = z;
    }

    void SetScale(float uniform) { SetScale(uniform, uniform, uniform); }
    void SetScale(Lightning::V3 s) { SetScale(s.x, s.y, s.z); }

    Lightning::V3 GetScale() const
    {
        return { transform.Scale.x, transform.Scale.y, transform.Scale.z };
    }

    // -----------------------------------------------------------------------
    // Direction helpers (derived from WorldMatrix)
    // -----------------------------------------------------------------------

    // Forward vector (–Z in local space, transformed to world).
    Lightning::V3 Forward() const
    {
        glm::mat4 wm  = WorldMatrix();
        glm::vec3 fwd = -glm::vec3(wm[2]);  // –Z column
        float len = glm::length(fwd);
        if (len < 0.0001f) return {0, 0, -1};
        fwd /= len;
        return { fwd.x, fwd.y, fwd.z };
    }

    // Right vector (+X in local space, transformed to world).
    Lightning::V3 Right() const
    {
        glm::mat4 wm  = WorldMatrix();
        glm::vec3 r   = glm::vec3(wm[0]);
        float len = glm::length(r);
        if (len < 0.0001f) return {1, 0, 0};
        r /= len;
        return { r.x, r.y, r.z };
    }

    // Up vector (+Y in local space, transformed to world).
    Lightning::V3 Up() const
    {
        glm::mat4 wm  = WorldMatrix();
        glm::vec3 u   = glm::vec3(wm[1]);
        float len = glm::length(u);
        if (len < 0.0001f) return {0, 1, 0};
        u /= len;
        return { u.x, u.y, u.z };
    }

    // Rotate so that –Z faces the target world position.
    void LookAt(Lightning::V3 target, Lightning::V3 up = {0, 1, 0})
    {
        Lightning::V3 wp = { WorldMatrix()[3][0], WorldMatrix()[3][1], WorldMatrix()[3][2] };
        glm::vec3 pos  = { wp.x,     wp.y,     wp.z     };
        glm::vec3 tgt  = { target.x, target.y, target.z };
        glm::vec3 upv  = { up.x,     up.y,     up.z     };

        glm::mat4 look = glm::lookAt(pos, tgt, upv);
        // Extract Euler angles (YXZ) from the view matrix (inverse look-at is model mat)
        glm::mat4 model = glm::inverse(look);
        transform.Rotation.x = glm::degrees(asinf(-model[2][1]));
        transform.Rotation.y = glm::degrees(atan2f(model[2][0], model[2][2]));
        transform.Rotation.z = 0.f;
    }

    // Distance to a world-space target position.
    float DistanceTo(Lightning::V3 target) const
    {
        glm::mat4 wm = WorldMatrix();
        float dx = target.x - wm[3][0];
        float dy = target.y - wm[3][1];
        float dz = target.z - wm[3][2];
        return sqrtf(dx*dx + dy*dy + dz*dz);
    }

    // Clone (copies transform, not components or children)
    std::unique_ptr<Node> Clone() const override
    {
        auto c = std::make_unique<Node3D>(name);
        c->tag       = tag;
        c->transform = transform;
        c->active    = active;
        return c;
    }
};

} // namespace LightningEngine
