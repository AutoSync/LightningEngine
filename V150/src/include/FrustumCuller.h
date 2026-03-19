// FrustumCuller.h — View-frustum culling for 3D scenes.
//
// Extracts the 6 clip planes from the combined ViewProjection matrix
// (Gribb-Hartmann method) and tests AABBs / spheres against them.
//
// Usage:
//   FrustumCuller culler;
//   culler.Extract(renderer.GetProjection() * renderer.GetView());
//
//   // Per object:
//   if (culler.TestAABB(node.BoundsMin(), node.BoundsMax()))
//       node.Render();
//
//   // Or sphere test:
//   if (culler.TestSphere(center, radius))
//       node.Render();
#pragma once
#include <glm/glm.hpp>
#include <array>

namespace LightningEngine {

class FrustumCuller {
public:
    // Extract 6 planes from a combined ViewProjection matrix.
    // Call once per frame after camera matrices are set.
    void Extract(const glm::mat4& vp)
    {
        // Gribb-Hartmann extraction — rows of the transposed VP matrix.
        // Each plane: ax + by + cz + d = 0, stored as vec4(a,b,c,d).
        glm::mat4 t = glm::transpose(vp);

        planes[Left]   = normalize(t[3] + t[0]);
        planes[Right]  = normalize(t[3] - t[0]);
        planes[Bottom] = normalize(t[3] + t[1]);
        planes[Top]    = normalize(t[3] - t[1]);
        planes[Near]   = normalize(t[3] + t[2]);
        planes[Far]    = normalize(t[3] - t[2]);
    }

    // Returns true if the AABB is at least partially inside the frustum.
    // min/max are world-space corners of the bounding box.
    bool TestAABB(const glm::vec3& mn, const glm::vec3& mx) const
    {
        for (const auto& pl : planes) {
            // Positive vertex (p-vertex): corner most in the direction of the normal.
            glm::vec3 pv;
            pv.x = (pl.x >= 0.f) ? mx.x : mn.x;
            pv.y = (pl.y >= 0.f) ? mx.y : mn.y;
            pv.z = (pl.z >= 0.f) ? mx.z : mn.z;
            // If p-vertex is on the negative side, AABB is fully outside.
            if (pl.x * pv.x + pl.y * pv.y + pl.z * pv.z + pl.w < 0.f)
                return false;
        }
        return true;
    }

    // Returns true if the sphere is at least partially inside the frustum.
    bool TestSphere(const glm::vec3& center, float radius) const
    {
        for (const auto& pl : planes) {
            float dist = pl.x * center.x + pl.y * center.y + pl.z * center.z + pl.w;
            if (dist < -radius)
                return false;
        }
        return true;
    }

    // Returns true if a point is inside the frustum.
    bool TestPoint(const glm::vec3& p) const
    {
        for (const auto& pl : planes) {
            if (pl.x * p.x + pl.y * p.y + pl.z * p.z + pl.w < 0.f)
                return false;
        }
        return true;
    }

private:
    enum PlaneIdx { Left, Right, Bottom, Top, Near, Far };
    std::array<glm::vec4, 6> planes;

    static glm::vec4 normalize(const glm::vec4& p)
    {
        float len = glm::length(glm::vec3(p));
        return (len > 0.f) ? p / len : p;
    }
};

} // namespace LightningEngine
