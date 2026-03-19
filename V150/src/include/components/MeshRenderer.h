// MeshRendererComponent — Renders a Mesh at the owner's world transform.
// Calls Renderer::DrawMesh() in Render() using the node's WorldMatrix().
//
// Usage:
//   auto* mr = node->AddComponent<MeshRendererComponent>();
//   mr->SetMesh(&cubeMesh);
//   mr->SetAlbedo(&woodTex);   // optional; white fallback used if nullptr
#pragma once
#include "../Component.h"
#include "../Mesh.h"
#include "../Texture.h"

namespace LightningEngine {

class MeshRendererComponent : public Component {
public:
    Mesh*    mesh   = nullptr;
    Texture* albedo = nullptr;

    void SetMesh(Mesh* m)      { mesh   = m; }
    void SetAlbedo(Texture* t) { albedo = t; }

    void Render() override
    {
        if (!mesh || !owner || !owner->renderer) return;
        owner->renderer->DrawMesh(*mesh, owner->WorldMatrix(), albedo);
    }
};

} // namespace LightningEngine
