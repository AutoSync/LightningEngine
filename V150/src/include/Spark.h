// Spark.h — Lightning Engine post-process shader system.
//
// .spark files contain vertex + fragment shader bodies, plus optional @inject
// directives that ask the engine to provide standard uniforms automatically.
//
// .spark format:
//   @inject Time          — fragment slot 1: vec4 spark_Time (x=sec, y=dt, z=sin(t), w=cos(t))
//   @inject Light         — fragment slot 2: vec4 spark_LightPos/Color
//   @inject Fog           — fragment slot 3: vec4 spark_FogColor/Params
//   @inject Transform     — vertex   slot 0: mat4 spark_Transform
//   @vertex               — vertex shader main() body follows
//   @fragment             — fragment shader main() body follows
//
// Always provided in fragment: spark_Texture (sampler2D, set=2 binding=0)
//                              spark_Tint    (vec4,      fragment slot 0)
// Always provided in vertex:   inPos (vec2), inUV (vec2), fragUV (out vec2)
//
// Workflow:
//   1. Write my.spark.
//   2. SparkCompiler::Compile("my.spark", "assets/shaders/my");
//      → my_vert.spv + my_frag.spv
//   3. SparkShader s; s.Load(renderer, "assets/shaders/my", flags);
//   4. Each frame: s.SetTime(t); s.Process(renderer, srcTex, destFB);
#pragma once
#include <cstdint>
#include <string>
#include "Renderer.h"
#include "Framebuffer.h"
#include "Texture.h"

namespace LightningEngine {

// ─────────────────────────────────────────────────────────────────────────────
// Inject flags — bitmask describing which engine uniforms are active
// ─────────────────────────────────────────────────────────────────────────────
enum SparkInjectFlags : uint32_t {
    SPARK_INJECT_NONE      = 0,
    SPARK_INJECT_TIME      = 1 << 0,   // spark_Time     (fragment slot 1)
    SPARK_INJECT_LIGHT     = 1 << 1,   // spark_Light*   (fragment slot 2)
    SPARK_INJECT_FOG       = 1 << 2,   // spark_Fog*     (fragment slot 3)
    SPARK_INJECT_TRANSFORM = 1 << 3,   // spark_Transform (vertex slot 0)
};

// ─────────────────────────────────────────────────────────────────────────────
// SparkShader — runtime post-process shader
// ─────────────────────────────────────────────────────────────────────────────
class SparkShader {
public:
    SparkShader()  = default;
    ~SparkShader() { Release(); }

    // Load pre-compiled SPIR-V. spvBase = path without the _vert.spv/_frag.spv suffix.
    // injectFlags must match the @inject directives used when compiling the shader.
    bool Load(Renderer& r, const char* spvBase,
              uint32_t injectFlags = SPARK_INJECT_NONE);

    void Release();
    bool IsValid() const { return pipeline != nullptr; }

    // Apply shader: renders src to dest using a full-screen quad.
    void Process(Renderer& r, Texture& src, Framebuffer& dest);

    // ── Uniform setters (call before Process) ────────────────────────────
    void SetTint(float r, float g, float b, float a = 1.f);
    void SetTime(float timeSec, float dt = 0.f);   // sin/cos auto-computed
    void SetLight(float posX, float posY, float intensity,
                  float cr, float cg, float cb);
    void SetFog(float cr, float cg, float cb,
                float density, float fogStart = 0.f, float fogEnd = 1.f);

private:
    SDL_GPUDevice*           device   = nullptr;
    SDL_GPUGraphicsPipeline* pipeline = nullptr;
    SDL_GPUBuffer*           vbuf     = nullptr;
    uint32_t                 flags    = 0;

    float tint[4]  = { 1.f, 1.f, 1.f, 1.f };
    float time[4]  = { 0.f, 0.f, 0.f, 0.f };
    // [0..1]=posXY, [2]=intensity, [3]=pad | [4..6]=colorRGB, [7]=pad
    float light[8] = {};
    // [0..2]=colorRGB, [3]=density | [4]=start, [5]=end, [6..7]=pad
    float fog[8]   = {};

    static SDL_GPUShader* loadSPV(SDL_GPUDevice* dev, const char* path,
                                   SDL_GPUShaderStage stage,
                                   uint32_t numSamplers,
                                   uint32_t numUniformBuffers);
    static SDL_GPUGraphicsPipeline* makePipeline(SDL_GPUDevice* dev,
                                                  SDL_GPUShader* vert,
                                                  SDL_GPUShader* frag);
};

// ─────────────────────────────────────────────────────────────────────────────
// SparkCompiler — offline .spark → GLSL → SPIR-V
// ─────────────────────────────────────────────────────────────────────────────
class SparkCompiler {
public:
    // Parse sparkPath, write generated GLSL to outVertPath and outFragPath.
    // Returns the inject flags bitmask, or 0xFFFFFFFF on parse error.
    static uint32_t GenerateGLSL(const char* sparkPath,
                                  const char* outVertPath,
                                  const char* outFragPath);

    // GenerateGLSL, then invoke glslc to compile → spvBase_vert.spv + spvBase_frag.spv.
    // Returns true on success. glslcExe should be "glslc" (requires Vulkan SDK in PATH).
    static bool Compile(const char* sparkPath,
                        const char* spvBase,
                        const char* glslcExe = "glslc");

private:
    struct ParseResult {
        uint32_t    flags    = 0;
        std::string vertBody;
        std::string fragBody;
        bool        ok       = false;
    };
    static ParseResult    parse(const char* path);
    static std::string    buildVert(const ParseResult& pr);
    static std::string    buildFrag(const ParseResult& pr);
    static bool           writeFile(const char* path, const std::string& src);
    static bool           runGlslc(const char* glslcExe,
                                   const char* inGlsl,
                                   const char* outSpv);
};

} // namespace LightningEngine
