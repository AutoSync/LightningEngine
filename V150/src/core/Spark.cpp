#include "../include/Spark.h"
#include <SDL3/SDL.h>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace LightningEngine {

// ============================================================================
// SparkShader — static helpers
// ============================================================================

SDL_GPUShader* SparkShader::loadSPV(SDL_GPUDevice* dev, const char* path,
                                     SDL_GPUShaderStage stage,
                                     uint32_t numSamplers,
                                     uint32_t numUniformBuffers)
{
    size_t sz  = 0;
    void*  code = SDL_LoadFile(path, &sz);
    if (!code) {
        std::cerr << "[Spark] Shader not found: " << path << "\n";
        return nullptr;
    }

    SDL_GPUShaderCreateInfo info = {};
    info.code                = (const Uint8*)code;
    info.code_size           = sz;
    info.entrypoint          = "main";
    info.format              = SDL_GPU_SHADERFORMAT_SPIRV;
    info.stage               = stage;
    info.num_samplers        = numSamplers;
    info.num_uniform_buffers = numUniformBuffers;

    SDL_GPUShader* shader = SDL_CreateGPUShader(dev, &info);
    if (!shader)
        std::cerr << "[Spark] SDL_CreateGPUShader failed: " << SDL_GetError() << "\n";

    SDL_free(code);
    return shader;
}

SDL_GPUGraphicsPipeline* SparkShader::makePipeline(SDL_GPUDevice* dev,
                                                    SDL_GPUShader* vert,
                                                    SDL_GPUShader* frag)
{
    // Vertex layout: vec2 inPos + vec2 inUV = 16 bytes/vertex
    SDL_GPUVertexBufferDescription vbDesc = {};
    vbDesc.slot       = 0;
    vbDesc.pitch      = sizeof(float) * 4;
    vbDesc.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;

    SDL_GPUVertexAttribute attrs[2] = {};
    attrs[0].location    = 0; attrs[0].buffer_slot = 0;
    attrs[0].format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
    attrs[0].offset      = 0;
    attrs[1].location    = 1; attrs[1].buffer_slot = 0;
    attrs[1].format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
    attrs[1].offset      = sizeof(float) * 2;

    SDL_GPUColorTargetBlendState blend = {};
    blend.enable_blend          = true;
    blend.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
    blend.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    blend.color_blend_op        = SDL_GPU_BLENDOP_ADD;
    blend.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
    blend.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    blend.alpha_blend_op        = SDL_GPU_BLENDOP_ADD;

    SDL_GPUColorTargetDescription ct = {};
    ct.format      = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    ct.blend_state = blend;

    SDL_GPUGraphicsPipelineCreateInfo pi = {};
    pi.vertex_shader                                 = vert;
    pi.fragment_shader                               = frag;
    pi.vertex_input_state.vertex_buffer_descriptions = &vbDesc;
    pi.vertex_input_state.num_vertex_buffers         = 1;
    pi.vertex_input_state.vertex_attributes          = attrs;
    pi.vertex_input_state.num_vertex_attributes      = 2;
    pi.primitive_type                                = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    pi.rasterizer_state.fill_mode                    = SDL_GPU_FILLMODE_FILL;
    pi.rasterizer_state.cull_mode                    = SDL_GPU_CULLMODE_NONE;
    pi.target_info.color_target_descriptions         = &ct;
    pi.target_info.num_color_targets                 = 1;

    return SDL_CreateGPUGraphicsPipeline(dev, &pi);
}

// ============================================================================
// SparkShader — public API
// ============================================================================

bool SparkShader::Load(Renderer& r, const char* spvBase, uint32_t injectFlags)
{
    Release();

    device = r.GetDevice();
    flags  = injectFlags;

    // Determine sampler/UBO counts
    uint32_t fragUBOs = 1; // always TintBlock (slot 0)
    if (flags & SPARK_INJECT_TIME)  fragUBOs++;
    if (flags & SPARK_INJECT_LIGHT) fragUBOs++;
    if (flags & SPARK_INJECT_FOG)   fragUBOs++;
    uint32_t vertUBOs = (flags & SPARK_INJECT_TRANSFORM) ? 1 : 0;

    std::string vertPath = std::string(spvBase) + "_vert.spv";
    std::string fragPath = std::string(spvBase) + "_frag.spv";

    SDL_GPUShader* vert = loadSPV(device, vertPath.c_str(),
                                   SDL_GPU_SHADERSTAGE_VERTEX,   0,       vertUBOs);
    SDL_GPUShader* frag = loadSPV(device, fragPath.c_str(),
                                   SDL_GPU_SHADERSTAGE_FRAGMENT, 1, fragUBOs);

    if (!vert || !frag) {
        if (vert) SDL_ReleaseGPUShader(device, vert);
        if (frag) SDL_ReleaseGPUShader(device, frag);
        device = nullptr;
        return false;
    }

    pipeline = makePipeline(device, vert, frag);
    SDL_ReleaseGPUShader(device, vert);
    SDL_ReleaseGPUShader(device, frag);

    if (!pipeline) {
        std::cerr << "[Spark] Pipeline creation failed: " << SDL_GetError() << "\n";
        device = nullptr;
        return false;
    }

    // Allocate vertex buffer for one full-screen quad (6 verts × 4 floats)
    SDL_GPUBufferCreateInfo bi = {};
    bi.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    bi.size  = 6 * sizeof(float) * 4;
    vbuf = SDL_CreateGPUBuffer(device, &bi);
    if (!vbuf) {
        SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
        pipeline = nullptr;
        device   = nullptr;
        return false;
    }

    return true;
}

void SparkShader::Release()
{
    if (device) {
        if (vbuf)     { SDL_ReleaseGPUBuffer(device, vbuf);                    vbuf     = nullptr; }
        if (pipeline) { SDL_ReleaseGPUGraphicsPipeline(device, pipeline); pipeline = nullptr; }
    }
    device = nullptr;
    flags  = 0;
}

void SparkShader::Process(Renderer& r, Texture& src, Framebuffer& dest)
{
    if (!pipeline || !vbuf || !dest.IsValid() || !src.IsValid()) return;

    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);
    if (!cmd) return;

    // Full-screen quad in NDC: x∈[-1,1], y∈[-1,1], uv∈[0,1]
    // Layout per vertex: x, y, u, v
    float quad[24] = {
        -1.f, -1.f, 0.f, 0.f,
         1.f, -1.f, 1.f, 0.f,
        -1.f,  1.f, 0.f, 1.f,
         1.f, -1.f, 1.f, 0.f,
         1.f,  1.f, 1.f, 1.f,
        -1.f,  1.f, 0.f, 1.f,
    };

    // Upload vertices
    Uint32 vSize = sizeof(quad);
    SDL_GPUTransferBufferCreateInfo tbi = { SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, vSize };
    SDL_GPUTransferBuffer* tb = SDL_CreateGPUTransferBuffer(device, &tbi);
    if (!tb) { SDL_SubmitGPUCommandBuffer(cmd); return; }

    SDL_memcpy(SDL_MapGPUTransferBuffer(device, tb, false), quad, vSize);
    SDL_UnmapGPUTransferBuffer(device, tb);

    SDL_GPUCopyPass* cp = SDL_BeginGPUCopyPass(cmd);
    SDL_GPUTransferBufferLocation tbl = { tb, 0 };
    SDL_GPUBufferRegion           dbr = { vbuf, 0, vSize };
    SDL_UploadToGPUBuffer(cp, &tbl, &dbr, true);
    SDL_EndGPUCopyPass(cp);
    SDL_ReleaseGPUTransferBuffer(device, tb);

    // Render pass → dest framebuffer
    SDL_GPUColorTargetInfo ct = {};
    ct.texture     = dest.GetGPUTexture();
    ct.clear_color = { 0.f, 0.f, 0.f, 0.f };
    ct.load_op     = SDL_GPU_LOADOP_CLEAR;
    ct.store_op    = SDL_GPU_STOREOP_STORE;

    SDL_GPURenderPass* rp = SDL_BeginGPURenderPass(cmd, &ct, 1, nullptr);
    SDL_BindGPUGraphicsPipeline(rp, pipeline);

    SDL_GPUBufferBinding vb = { vbuf, 0 };
    SDL_BindGPUVertexBuffers(rp, 0, &vb, 1);

    SDL_GPUTextureSamplerBinding tsb = { src.GetGPUTexture(), src.GetSampler() };
    SDL_BindGPUFragmentSamplers(rp, 0, &tsb, 1);

    // Slot 0: Tint (always)
    SDL_PushGPUFragmentUniformData(cmd, 0, tint, sizeof(tint));

    // Additional injected uniforms (slots 1, 2, 3)
    uint32_t slot = 1;
    if (flags & SPARK_INJECT_TIME)  SDL_PushGPUFragmentUniformData(cmd, slot++, time,  sizeof(time));
    if (flags & SPARK_INJECT_LIGHT) SDL_PushGPUFragmentUniformData(cmd, slot++, light, sizeof(light));
    if (flags & SPARK_INJECT_FOG)   SDL_PushGPUFragmentUniformData(cmd, slot++, fog,   sizeof(fog));

    if (flags & SPARK_INJECT_TRANSFORM) {
        // Identity matrix as default (caller sets it via a separate mechanism if needed)
        float identity[16] = {
            1,0,0,0,
            0,1,0,0,
            0,0,1,0,
            0,0,0,1
        };
        SDL_PushGPUVertexUniformData(cmd, 0, identity, sizeof(identity));
    }

    SDL_DrawGPUPrimitives(rp, 6, 1, 0, 0);
    SDL_EndGPURenderPass(rp);
    SDL_SubmitGPUCommandBuffer(cmd);
    SDL_WaitForGPUIdle(device);
}

void SparkShader::SetTint(float r, float g, float b, float a)
{
    tint[0] = r; tint[1] = g; tint[2] = b; tint[3] = a;
}

void SparkShader::SetTime(float timeSec, float dt)
{
    time[0] = timeSec;
    time[1] = dt;
    time[2] = std::sin(timeSec);
    time[3] = std::cos(timeSec);
}

void SparkShader::SetLight(float px, float py, float intensity,
                            float cr, float cg, float cb)
{
    light[0] = px;        light[1] = py;
    light[2] = intensity; light[3] = 0.f;
    light[4] = cr;        light[5] = cg;
    light[6] = cb;        light[7] = 0.f;
}

void SparkShader::SetFog(float cr, float cg, float cb,
                          float density, float fogStart, float fogEnd)
{
    fog[0] = cr;       fog[1] = cg;
    fog[2] = cb;       fog[3] = density;
    fog[4] = fogStart; fog[5] = fogEnd;
    fog[6] = 0.f;      fog[7] = 0.f;
}

// ============================================================================
// SparkCompiler — parse + GLSL generation
// ============================================================================

SparkCompiler::ParseResult SparkCompiler::parse(const char* path)
{
    ParseResult pr;
    std::ifstream f(path);
    if (!f) {
        std::cerr << "[SparkCompiler] Cannot open: " << path << "\n";
        return pr;
    }

    enum Section { NONE, VERT, FRAG } sec = NONE;
    std::ostringstream vertSS, fragSS;

    std::string line;
    while (std::getline(f, line)) {
        // Trim leading whitespace for directive matching
        std::string trimmed = line;
        while (!trimmed.empty() && (trimmed.front() == ' ' || trimmed.front() == '\t'))
            trimmed.erase(trimmed.begin());

        if (trimmed.substr(0, 8)  == "@inject ") {
            std::string tag = trimmed.substr(8);
            while (!tag.empty() && (tag.back() == ' ' || tag.back() == '\r')) tag.pop_back();
            if      (tag == "Time")      pr.flags |= SPARK_INJECT_TIME;
            else if (tag == "Light")     pr.flags |= SPARK_INJECT_LIGHT;
            else if (tag == "Fog")       pr.flags |= SPARK_INJECT_FOG;
            else if (tag == "Transform") pr.flags |= SPARK_INJECT_TRANSFORM;
            else std::cerr << "[SparkCompiler] Unknown inject: " << tag << "\n";
        } else if (trimmed == "@vertex") {
            sec = VERT;
        } else if (trimmed == "@fragment") {
            sec = FRAG;
        } else {
            if (sec == VERT) vertSS << line << "\n";
            if (sec == FRAG) fragSS << line << "\n";
        }
    }

    pr.vertBody = vertSS.str();
    pr.fragBody = fragSS.str();
    pr.ok       = !pr.vertBody.empty() && !pr.fragBody.empty();
    if (!pr.ok)
        std::cerr << "[SparkCompiler] Missing @vertex or @fragment section in: " << path << "\n";
    return pr;
}

std::string SparkCompiler::buildVert(const ParseResult& pr)
{
    std::ostringstream o;
    o << "#version 450\n\n";
    o << "layout(location = 0) in vec2 inPos;\n";
    o << "layout(location = 1) in vec2 inUV;\n";
    o << "layout(location = 0) out vec2 fragUV;\n";

    if (pr.flags & SPARK_INJECT_TRANSFORM)
        o << "\nlayout(set = 1, binding = 0) uniform TransformBlock {\n"
             "    mat4 spark_Transform;\n"
             "};\n";

    o << "\n" << pr.vertBody;
    return o.str();
}

std::string SparkCompiler::buildFrag(const ParseResult& pr)
{
    std::ostringstream o;
    o << "#version 450\n\n";
    o << "layout(location = 0) in vec2 fragUV;\n";
    o << "layout(location = 0) out vec4 outColor;\n\n";

    // Sampler (set=2 binding=0)
    o << "layout(set = 2, binding = 0) uniform sampler2D spark_Texture;\n\n";

    // Tint (fragment slot 0 → set=3 binding=0)
    o << "layout(set = 3, binding = 0) uniform TintBlock {\n"
         "    vec4 spark_Tint;\n"
         "};\n";

    // Time (fragment slot 1 → set=3 binding=1)
    if (pr.flags & SPARK_INJECT_TIME)
        o << "\nlayout(set = 3, binding = 1) uniform TimeBlock {\n"
             "    // x=timeSec, y=deltaTime, z=sin(t), w=cos(t)\n"
             "    vec4 spark_Time;\n"
             "};\n";

    // Light (fragment slot 2 → set=3 binding=2)
    if (pr.flags & SPARK_INJECT_LIGHT)
        o << "\nlayout(set = 3, binding = 2) uniform LightBlock {\n"
             "    vec4 spark_LightPos;   // xy=worldPos, z=intensity, w=unused\n"
             "    vec4 spark_LightColor; // rgb=color, a=unused\n"
             "};\n";

    // Fog (fragment slot 3 → set=3 binding=3)
    if (pr.flags & SPARK_INJECT_FOG)
        o << "\nlayout(set = 3, binding = 3) uniform FogBlock {\n"
             "    vec4 spark_FogColor;  // rgb=color, a=unused\n"
             "    vec4 spark_FogParams; // x=density, y=start, z=end, w=unused\n"
             "};\n";

    o << "\n" << pr.fragBody;
    return o.str();
}

bool SparkCompiler::writeFile(const char* path, const std::string& src)
{
    std::ofstream f(path);
    if (!f) { std::cerr << "[SparkCompiler] Cannot write: " << path << "\n"; return false; }
    f << src;
    return true;
}

bool SparkCompiler::runGlslc(const char* glslcExe, const char* inGlsl, const char* outSpv)
{
    std::string cmd = std::string(glslcExe) + " \"" + inGlsl + "\" -o \"" + outSpv + "\"";
    int ret = std::system(cmd.c_str());
    if (ret != 0)
        std::cerr << "[SparkCompiler] glslc failed (exit " << ret << ") for: " << inGlsl << "\n";
    return ret == 0;
}

uint32_t SparkCompiler::GenerateGLSL(const char* sparkPath,
                                      const char* outVertPath,
                                      const char* outFragPath)
{
    ParseResult pr = parse(sparkPath);
    if (!pr.ok) return 0xFFFFFFFF;

    if (!writeFile(outVertPath, buildVert(pr))) return 0xFFFFFFFF;
    if (!writeFile(outFragPath, buildFrag(pr))) return 0xFFFFFFFF;

    return pr.flags;
}

bool SparkCompiler::Compile(const char* sparkPath,
                             const char* spvBase,
                             const char* glslcExe)
{
    ParseResult pr = parse(sparkPath);
    if (!pr.ok) return false;

    // Write temporary GLSL files alongside the .spark
    std::string vertGlsl = std::string(spvBase) + "_vert.glsl";
    std::string fragGlsl = std::string(spvBase) + "_frag.glsl";

    if (!writeFile(vertGlsl.c_str(), buildVert(pr))) return false;
    if (!writeFile(fragGlsl.c_str(), buildFrag(pr))) return false;

    std::string vertSpv = std::string(spvBase) + "_vert.spv";
    std::string fragSpv = std::string(spvBase) + "_frag.spv";

    bool ok = runGlslc(glslcExe, vertGlsl.c_str(), vertSpv.c_str())
           && runGlslc(glslcExe, fragGlsl.c_str(), fragSpv.c_str());

    // Clean up temp GLSL
    std::remove(vertGlsl.c_str());
    std::remove(fragGlsl.c_str());

    if (ok)
        std::cout << "[SparkCompiler] Compiled: " << spvBase << "_vert.spv + _frag.spv\n";

    return ok;
}

} // namespace LightningEngine
