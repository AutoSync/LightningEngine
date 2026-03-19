// Mesh.h — GPU mesh data, built-in primitives, and Assimp model loader.
//
// Vertex layout (32 bytes, interleaved):
//   float[3] position   (offset  0)
//   float[3] normal     (offset 12)
//   float[2] uv         (offset 24)
//
// Usage:
//   Mesh cube = MeshPrimitive::Cube();
//   renderer.UploadMesh(cube);          // upload once
//   renderer.DrawMesh(cube, modelMat);  // per frame
//   renderer.ReleaseMesh(cube);         // on shutdown
//
// MODEL LOADING:
//   auto meshes = ModelLoader::Load("assets/character.fbx");
//   for (auto& m : meshes) renderer.UploadMesh(m);
#pragma once
#include <vector>
#include <string>
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

namespace LightningEngine {

// ─────────────────────────────────────────────────────────────────────────────
// Mesh
// ─────────────────────────────────────────────────────────────────────────────
struct Mesh {
    // CPU-side geometry (kept after upload for CPU queries / re-upload)
    std::vector<float>  vertices;   // pos(3)+normal(3)+uv(2) = 8 floats per vertex
    std::vector<Uint32> indices;

    // GPU buffers — valid after Renderer::UploadMesh(), null before
    SDL_GPUBuffer* vbuf = nullptr;
    SDL_GPUBuffer* ibuf = nullptr;

    bool   IsValid()     const { return vbuf != nullptr && ibuf != nullptr; }
    Uint32 IndexCount()  const { return static_cast<Uint32>(indices.size()); }
    Uint32 VertexCount() const { return static_cast<Uint32>(vertices.size() / 8); }
};

// ─────────────────────────────────────────────────────────────────────────────
// Built-in primitives (CPU geometry, no GPU state — pass to UploadMesh)
// ─────────────────────────────────────────────────────────────────────────────
namespace MeshPrimitive {
    Mesh Cube();
    Mesh Plane(int subdiv = 1);
    Mesh Sphere(int rings = 16, int sectors = 16);
}

// ─────────────────────────────────────────────────────────────────────────────
// Assimp model loader
// ─────────────────────────────────────────────────────────────────────────────
namespace ModelLoader {
    // Load all meshes from a file. Returns empty vector on failure.
    // Post-processes: triangulate, gen smooth normals, flip UVs.
    std::vector<Mesh> Load(const char* path);
}

} // namespace LightningEngine
