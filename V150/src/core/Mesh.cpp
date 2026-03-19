#include "../include/Mesh.h"
#include <array>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>

namespace LightningEngine {

// ─────────────────────────────────────────────────────────────────────────────
// MeshPrimitive
// ─────────────────────────────────────────────────────────────────────────────

namespace MeshPrimitive {

Mesh Cube()
{
    Mesh m;
    // 24 verts (4 per face, 6 faces) — separate faces so normals are per-face
    static const float v[] = {
        // pos(3)          normal(3)    uv(2)
        // Front (+Z)
        -0.5f,-0.5f, 0.5f,  0,0,1,  0,1,
         0.5f,-0.5f, 0.5f,  0,0,1,  1,1,
         0.5f, 0.5f, 0.5f,  0,0,1,  1,0,
        -0.5f, 0.5f, 0.5f,  0,0,1,  0,0,
        // Back (-Z)
         0.5f,-0.5f,-0.5f,  0,0,-1,  0,1,
        -0.5f,-0.5f,-0.5f,  0,0,-1,  1,1,
        -0.5f, 0.5f,-0.5f,  0,0,-1,  1,0,
         0.5f, 0.5f,-0.5f,  0,0,-1,  0,0,
        // Right (+X)
         0.5f,-0.5f, 0.5f,  1,0,0,  0,1,
         0.5f,-0.5f,-0.5f,  1,0,0,  1,1,
         0.5f, 0.5f,-0.5f,  1,0,0,  1,0,
         0.5f, 0.5f, 0.5f,  1,0,0,  0,0,
        // Left (-X)
        -0.5f,-0.5f,-0.5f, -1,0,0,  0,1,
        -0.5f,-0.5f, 0.5f, -1,0,0,  1,1,
        -0.5f, 0.5f, 0.5f, -1,0,0,  1,0,
        -0.5f, 0.5f,-0.5f, -1,0,0,  0,0,
        // Top (+Y)
        -0.5f, 0.5f, 0.5f,  0,1,0,  0,1,
         0.5f, 0.5f, 0.5f,  0,1,0,  1,1,
         0.5f, 0.5f,-0.5f,  0,1,0,  1,0,
        -0.5f, 0.5f,-0.5f,  0,1,0,  0,0,
        // Bottom (-Y)
        -0.5f,-0.5f,-0.5f,  0,-1,0,  0,1,
         0.5f,-0.5f,-0.5f,  0,-1,0,  1,1,
         0.5f,-0.5f, 0.5f,  0,-1,0,  1,0,
        -0.5f,-0.5f, 0.5f,  0,-1,0,  0,0,
    };
    m.vertices.insert(m.vertices.end(), v, v + sizeof(v)/sizeof(float));
    for (int f = 0; f < 6; f++) {
        Uint32 b = f * 4;
        m.indices.insert(m.indices.end(), { b, b+1, b+2,  b, b+2, b+3 });
    }
    return m;
}

Mesh Plane(int subdiv)
{
    if (subdiv < 1) subdiv = 1;
    Mesh m;
    float step = 1.f / subdiv;
    for (int z = 0; z <= subdiv; z++) {
        for (int x = 0; x <= subdiv; x++) {
            float px = x * step - 0.5f;
            float pz = z * step - 0.5f;
            m.vertices.insert(m.vertices.end(),
                { px, 0.f, pz,  0.f, 1.f, 0.f,  x * step, z * step });
        }
    }
    int w = subdiv + 1;
    for (int z = 0; z < subdiv; z++) {
        for (int x = 0; x < subdiv; x++) {
            Uint32 tl = z*w+x, tr = tl+1, bl = tl+w, br = bl+1;
            m.indices.insert(m.indices.end(), { tl, bl, tr,  tr, bl, br });
        }
    }
    return m;
}

Mesh Sphere(int rings, int sectors)
{
    if (rings   < 4) rings   = 4;
    if (sectors < 4) sectors = 4;
    constexpr float PI  = 3.14159265f;
    constexpr float PI2 = 6.28318530f;
    Mesh m;
    for (int r = 0; r <= rings; r++) {
        float phi = PI * r / rings;
        float sp  = sinf(phi), cp = cosf(phi);
        for (int s = 0; s <= sectors; s++) {
            float theta = PI2 * s / sectors;
            float st = sinf(theta), ct = cosf(theta);
            float px = sp*ct, py = cp, pz = sp*st;
            m.vertices.insert(m.vertices.end(),
                { px, py, pz,  px, py, pz,
                  (float)s/sectors, (float)r/rings });
        }
    }
    int w = sectors + 1;
    for (int r = 0; r < rings; r++) {
        for (int s = 0; s < sectors; s++) {
            Uint32 tl = r*w+s, tr = tl+1, bl = tl+w, br = bl+1;
            m.indices.insert(m.indices.end(), { tl, tr, bl,  tr, br, bl });
        }
    }
    return m;
}

} // namespace MeshPrimitive

// ─────────────────────────────────────────────────────────────────────────────
// ModelLoader — self-contained .obj parser (no Assimp dependency)
//
// Supports: v, vn, vt, f (triangles and polygon fans), o/g grouping.
// Vertex deduplication via hash map on (posIdx, uvIdx, normIdx) triplets.
// ─────────────────────────────────────────────────────────────────────────────
namespace ModelLoader {

namespace {

struct OBJKey {
    int pi, ti, ni;
    bool operator==(const OBJKey& o) const
    { return pi == o.pi && ti == o.ti && ni == o.ni; }
};

struct OBJKeyHash {
    size_t operator()(const OBJKey& k) const
    {
        size_t h = (size_t)k.pi;
        h ^= (size_t)k.ti * 2654435761u + 0x9e3779b9u + (h << 6) + (h >> 2);
        h ^= (size_t)k.ni * 2246822519u + 0x9e3779b9u + (h << 6) + (h >> 2);
        return h;
    }
};

// Wrap a possibly-negative OBJ index to a zero-based positive index.
static int resolveIdx(int raw, int count)
{
    if (raw < 0) return count + raw;
    return raw - 1; // OBJ indices are 1-based
}

// Parse a face vertex token "v", "v/vt", "v//vn", or "v/vt/vn".
static OBJKey parseFaceVert(const std::string& tok, int np, int nt, int nn)
{
    OBJKey k { 0, -1, -1 };
    int slash1 = (int)tok.find('/');
    if (slash1 == (int)std::string::npos) {
        k.pi = resolveIdx(std::stoi(tok), np);
    } else {
        k.pi = resolveIdx(std::stoi(tok.substr(0, slash1)), np);
        int slash2 = (int)tok.find('/', slash1 + 1);
        if (slash2 == (int)std::string::npos) {
            // v/vt
            if (slash1 + 1 < (int)tok.size())
                k.ti = resolveIdx(std::stoi(tok.substr(slash1 + 1)), nt);
        } else {
            // v/vt/vn or v//vn
            if (slash2 > slash1 + 1)
                k.ti = resolveIdx(std::stoi(tok.substr(slash1 + 1, slash2 - slash1 - 1)), nt);
            if (slash2 + 1 < (int)tok.size())
                k.ni = resolveIdx(std::stoi(tok.substr(slash2 + 1)), nn);
        }
    }
    return k;
}

} // anonymous namespace

std::vector<Mesh> Load(const char* path)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "[ModelLoader] Cannot open: " << path << "\n";
        return {};
    }

    std::vector<std::array<float,3>> pos;
    std::vector<std::array<float,3>> norms;
    std::vector<std::array<float,2>> uvs;

    // We build one Mesh per 'o'/'g' group (or the whole file if no groups).
    struct Group {
        std::string name;
        std::unordered_map<OBJKey, Uint32, OBJKeyHash> vertCache;
        Mesh mesh;
    };

    std::vector<Group> groups;
    groups.emplace_back();
    groups.back().name = "default";

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream ss(line);
        std::string token;
        ss >> token;

        if (token == "v") {
            float x, y, z;
            ss >> x >> y >> z;
            pos.push_back({x, y, z});
        }
        else if (token == "vn") {
            float x, y, z;
            ss >> x >> y >> z;
            norms.push_back({x, y, z});
        }
        else if (token == "vt") {
            float u, v;
            ss >> u >> v;
            uvs.push_back({u, 1.f - v}); // flip V for Vulkan
        }
        else if (token == "o" || token == "g") {
            std::string gname;
            ss >> gname;
            // Start a new group only if the current group has geometry.
            if (!groups.back().mesh.vertices.empty() ||
                !groups.back().mesh.indices.empty()) {
                groups.emplace_back();
                groups.back().name = gname;
            } else {
                groups.back().name = gname;
            }
        }
        else if (token == "f") {
            // Collect all face vertex tokens on this line.
            std::vector<OBJKey> faceVerts;
            std::string ftok;
            while (ss >> ftok)
                faceVerts.push_back(parseFaceVert(ftok,
                    (int)pos.size(), (int)uvs.size(), (int)norms.size()));

            // Fan-triangulate: (0,1,2), (0,2,3), (0,3,4), ...
            if (faceVerts.size() < 3) continue;

            Group& g = groups.back();
            auto addVert = [&](const OBJKey& k) -> Uint32 {
                auto it = g.vertCache.find(k);
                if (it != g.vertCache.end()) return it->second;

                Uint32 idx = (Uint32)(g.mesh.vertices.size() / 8);

                float px = 0, py = 0, pz = 0;
                if (k.pi >= 0 && k.pi < (int)pos.size()) {
                    px = pos[k.pi][0]; py = pos[k.pi][1]; pz = pos[k.pi][2];
                }
                float nx = 0, ny = 1, nz = 0;
                if (k.ni >= 0 && k.ni < (int)norms.size()) {
                    nx = norms[k.ni][0]; ny = norms[k.ni][1]; nz = norms[k.ni][2];
                }
                float u = 0, v = 0;
                if (k.ti >= 0 && k.ti < (int)uvs.size()) {
                    u = uvs[k.ti][0]; v = uvs[k.ti][1];
                }
                g.mesh.vertices.insert(g.mesh.vertices.end(),
                    { px, py, pz, nx, ny, nz, u, v });
                g.vertCache[k] = idx;
                return idx;
            };

            Uint32 i0 = addVert(faceVerts[0]);
            for (size_t i = 1; i + 1 < faceVerts.size(); i++) {
                Uint32 i1 = addVert(faceVerts[i]);
                Uint32 i2 = addVert(faceVerts[i + 1]);
                g.mesh.indices.insert(g.mesh.indices.end(), { i0, i1, i2 });
            }
        }
    }

    // Collect non-empty meshes.
    std::vector<Mesh> result;
    for (auto& g : groups) {
        if (!g.mesh.vertices.empty() && !g.mesh.indices.empty())
            result.push_back(std::move(g.mesh));
    }

    if (result.empty())
        std::cerr << "[ModelLoader] No geometry found in: " << path << "\n";
    else
        std::cout << "[ModelLoader] Loaded " << result.size()
                  << " mesh(es) from: " << path << "\n";
    return result;
}

} // namespace ModelLoader

} // namespace LightningEngine
