#include "../include/Renderer.h"
#include <cmath>
#include <iostream>

namespace LightningEngine {

static constexpr float kPI = 3.14159265f;

// ============================================================================
// Constructor
// ============================================================================

Renderer::Renderer(SDL_GPUDevice* dev, SDL_Window* win)
	: device(dev), window(win)
{
	swapFmt = SDL_GetGPUSwapchainTextureFormat(device, window);
	initPipeline2D();
	initPipelineTex();
	initPipelinesRT();
	initPipelineBlur();
	initPipeline3D();

	// 1×1 white fallback for 3D meshes without an albedo
	static const Uint8 white[4] = { 255, 255, 255, 255 };
	whiteTex = Texture::FromPixels(device, white, 1, 1);
}

// ============================================================================
// Shader loading
// ============================================================================

SDL_GPUShader* Renderer::loadShader(SDL_GPUShaderStage stage, const char* path,
                                     Uint32 numSamplers, Uint32 numUniformBuffers)
{
	size_t size = 0;
	void*  code = SDL_LoadFile(path, &size);
	if (!code) {
		std::cerr << "[Renderer] Shader not found: " << path << "\n";
		return nullptr;
	}

	SDL_GPUShaderCreateInfo info = {};
	info.code                = (const Uint8*)code;
	info.code_size           = size;
	info.entrypoint          = "main";
	info.format              = SDL_GPU_SHADERFORMAT_SPIRV;
	info.stage               = stage;
	info.num_samplers        = numSamplers;
	info.num_uniform_buffers = numUniformBuffers;

	SDL_GPUShader* shader = SDL_CreateGPUShader(device, &info);
	if (!shader)
		std::cerr << "[Renderer] SDL_CreateGPUShader failed: " << SDL_GetError() << "\n";

	SDL_free(code);
	return shader;
}

// ============================================================================
// Pipeline factory helpers
// ============================================================================

SDL_GPUGraphicsPipeline* Renderer::makePipeline2D(SDL_GPUTextureFormat fmt,
                                                    SDL_GPUShader* vert,
                                                    SDL_GPUShader* frag)
{
	SDL_GPUVertexBufferDescription vbDesc = {};
	vbDesc.slot       = 0;
	vbDesc.pitch      = sizeof(float) * 2;
	vbDesc.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;

	SDL_GPUVertexAttribute attr = {};
	attr.location    = 0;
	attr.buffer_slot = 0;
	attr.format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
	attr.offset      = 0;

	SDL_GPUColorTargetBlendState blend = {};
	blend.enable_blend             = true;
	blend.src_color_blendfactor    = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
	blend.dst_color_blendfactor    = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
	blend.color_blend_op           = SDL_GPU_BLENDOP_ADD;
	blend.src_alpha_blendfactor    = SDL_GPU_BLENDFACTOR_ONE;
	blend.dst_alpha_blendfactor    = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
	blend.alpha_blend_op           = SDL_GPU_BLENDOP_ADD;

	SDL_GPUColorTargetDescription colorTarget = {};
	colorTarget.format      = fmt;
	colorTarget.blend_state = blend;

	SDL_GPUGraphicsPipelineCreateInfo pi = {};
	pi.vertex_shader                                 = vert;
	pi.fragment_shader                               = frag;
	pi.vertex_input_state.vertex_buffer_descriptions = &vbDesc;
	pi.vertex_input_state.num_vertex_buffers         = 1;
	pi.vertex_input_state.vertex_attributes          = &attr;
	pi.vertex_input_state.num_vertex_attributes      = 1;
	pi.primitive_type                                = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
	pi.rasterizer_state.fill_mode                    = SDL_GPU_FILLMODE_FILL;
	pi.rasterizer_state.cull_mode                    = SDL_GPU_CULLMODE_NONE;
	pi.target_info.color_target_descriptions         = &colorTarget;
	pi.target_info.num_color_targets                 = 1;

	return SDL_CreateGPUGraphicsPipeline(device, &pi);
}

SDL_GPUGraphicsPipeline* Renderer::makePipelineTex(SDL_GPUTextureFormat fmt,
                                                     SDL_GPUShader* vert,
                                                     SDL_GPUShader* frag)
{
	SDL_GPUVertexBufferDescription vbDesc = {};
	vbDesc.slot       = 0;
	vbDesc.pitch      = sizeof(float) * 4;
	vbDesc.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;

	SDL_GPUVertexAttribute attrs[2] = {};
	attrs[0].location    = 0; attrs[0].buffer_slot = 0;
	attrs[0].format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2; attrs[0].offset = 0;
	attrs[1].location    = 1; attrs[1].buffer_slot = 0;
	attrs[1].format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
	attrs[1].offset      = sizeof(float) * 2;

	SDL_GPUColorTargetBlendState blend = {};
	blend.enable_blend             = true;
	blend.src_color_blendfactor    = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
	blend.dst_color_blendfactor    = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
	blend.color_blend_op           = SDL_GPU_BLENDOP_ADD;
	blend.src_alpha_blendfactor    = SDL_GPU_BLENDFACTOR_ONE;
	blend.dst_alpha_blendfactor    = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
	blend.alpha_blend_op           = SDL_GPU_BLENDOP_ADD;

	SDL_GPUColorTargetDescription colorTarget = {};
	colorTarget.format      = fmt;
	colorTarget.blend_state = blend;

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
	pi.target_info.color_target_descriptions         = &colorTarget;
	pi.target_info.num_color_targets                 = 1;

	return SDL_CreateGPUGraphicsPipeline(device, &pi);
}

SDL_GPUGraphicsPipeline* Renderer::makePipeline3D(SDL_GPUTextureFormat fmt,
                                                    SDL_GPUShader* vert,
                                                    SDL_GPUShader* frag)
{
	// Vertex layout: float3 pos (loc 0), float3 normal (loc 1), float2 uv (loc 2)
	SDL_GPUVertexBufferDescription vbDesc = {};
	vbDesc.slot       = 0;
	vbDesc.pitch      = sizeof(float) * 8;  // 32 bytes per vertex
	vbDesc.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;

	SDL_GPUVertexAttribute attrs[3] = {};
	attrs[0].location = 0; attrs[0].buffer_slot = 0;
	attrs[0].format   = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3; attrs[0].offset = 0;
	attrs[1].location = 1; attrs[1].buffer_slot = 0;
	attrs[1].format   = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3; attrs[1].offset = sizeof(float)*3;
	attrs[2].location = 2; attrs[2].buffer_slot = 0;
	attrs[2].format   = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2; attrs[2].offset = sizeof(float)*6;

	SDL_GPUColorTargetBlendState blend = {};
	blend.enable_blend          = false;

	SDL_GPUColorTargetDescription colorTarget = {};
	colorTarget.format      = fmt;
	colorTarget.blend_state = blend;

	SDL_GPUDepthStencilState ds = {};
	ds.enable_depth_test   = true;
	ds.enable_depth_write  = true;
	ds.compare_op          = SDL_GPU_COMPAREOP_LESS;

	SDL_GPUGraphicsPipelineCreateInfo pi = {};
	pi.vertex_shader                                 = vert;
	pi.fragment_shader                               = frag;
	pi.vertex_input_state.vertex_buffer_descriptions = &vbDesc;
	pi.vertex_input_state.num_vertex_buffers         = 1;
	pi.vertex_input_state.vertex_attributes          = attrs;
	pi.vertex_input_state.num_vertex_attributes      = 3;
	pi.primitive_type                                = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
	pi.rasterizer_state.fill_mode                    = SDL_GPU_FILLMODE_FILL;
	pi.rasterizer_state.cull_mode                    = SDL_GPU_CULLMODE_BACK;
	pi.rasterizer_state.front_face                   = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
	pi.depth_stencil_state                           = ds;
	pi.target_info.color_target_descriptions         = &colorTarget;
	pi.target_info.num_color_targets                 = 1;
	pi.target_info.depth_stencil_format              = SDL_GPU_TEXTUREFORMAT_D16_UNORM;
	pi.target_info.has_depth_stencil_target          = true;

	return SDL_CreateGPUGraphicsPipeline(device, &pi);
}

// ============================================================================
// Pipeline init
// ============================================================================

bool Renderer::initPipeline2D()
{
	if (!device || !window) return false;
	SDL_GPUShader* vert = loadShader(SDL_GPU_SHADERSTAGE_VERTEX,
	                                  "assets/shaders/rect2d_vert.spv", 0, 0);
	SDL_GPUShader* frag = loadShader(SDL_GPU_SHADERSTAGE_FRAGMENT,
	                                  "assets/shaders/rect2d_frag.spv", 0, 1);
	if (!vert || !frag) {
		if (vert) SDL_ReleaseGPUShader(device, vert);
		if (frag) SDL_ReleaseGPUShader(device, frag);
		std::cerr << "[Renderer] Colored 2D pipeline unavailable.\n";
		return false;
	}

	pipe2D = makePipeline2D(swapFmt, vert, frag);
	SDL_ReleaseGPUShader(device, vert);
	SDL_ReleaseGPUShader(device, frag);
	if (!pipe2D) { std::cerr << "[Renderer] pipe2D failed.\n"; return false; }

	SDL_GPUBufferCreateInfo bi = {};
	bi.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
	bi.size  = kMaxVerts * sizeof(float) * 2;
	vbuf2D = SDL_CreateGPUBuffer(device, &bi);
	return vbuf2D != nullptr;
}

bool Renderer::initPipelineTex()
{
	if (!device || !window) return false;
	SDL_GPUShader* vert = loadShader(SDL_GPU_SHADERSTAGE_VERTEX,
	                                  "assets/shaders/rect2d_tex_vert.spv", 0, 0);
	SDL_GPUShader* frag = loadShader(SDL_GPU_SHADERSTAGE_FRAGMENT,
	                                  "assets/shaders/rect2d_tex_frag.spv", 1, 1);
	if (!vert || !frag) {
		if (vert) SDL_ReleaseGPUShader(device, vert);
		if (frag) SDL_ReleaseGPUShader(device, frag);
		std::cerr << "[Renderer] Textured 2D pipeline unavailable.\n";
		return false;
	}

	pipeTex = makePipelineTex(swapFmt, vert, frag);
	SDL_ReleaseGPUShader(device, vert);
	SDL_ReleaseGPUShader(device, frag);
	if (!pipeTex) { std::cerr << "[Renderer] pipeTex failed.\n"; return false; }

	SDL_GPUBufferCreateInfo bi = {};
	bi.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
	bi.size  = kMaxTexVerts * sizeof(float) * 4;
	vbufTex = SDL_CreateGPUBuffer(device, &bi);
	return vbufTex != nullptr;
}

bool Renderer::initPipelinesRT()
{
	if (!device || !window) return false;

	constexpr SDL_GPUTextureFormat kRTFmt = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;

	SDL_GPUShader* v2d  = loadShader(SDL_GPU_SHADERSTAGE_VERTEX,
	                                  "assets/shaders/rect2d_vert.spv", 0, 0);
	SDL_GPUShader* f2d  = loadShader(SDL_GPU_SHADERSTAGE_FRAGMENT,
	                                  "assets/shaders/rect2d_frag.spv", 0, 1);
	SDL_GPUShader* vtex = loadShader(SDL_GPU_SHADERSTAGE_VERTEX,
	                                  "assets/shaders/rect2d_tex_vert.spv", 0, 0);
	SDL_GPUShader* ftex = loadShader(SDL_GPU_SHADERSTAGE_FRAGMENT,
	                                  "assets/shaders/rect2d_tex_frag.spv", 1, 1);

	if (v2d && f2d) {
		pipe2D_rt = makePipeline2D(kRTFmt, v2d, f2d);
		if (!pipe2D_rt) std::cerr << "[Renderer] pipe2D_rt failed.\n";
	}
	if (vtex && ftex) {
		pipeTex_rt = makePipelineTex(kRTFmt, vtex, ftex);
		if (!pipeTex_rt) std::cerr << "[Renderer] pipeTex_rt failed.\n";
	}

	if (v2d)  SDL_ReleaseGPUShader(device, v2d);
	if (f2d)  SDL_ReleaseGPUShader(device, f2d);
	if (vtex) SDL_ReleaseGPUShader(device, vtex);
	if (ftex) SDL_ReleaseGPUShader(device, ftex);

	if (!pipe2D_rt || !pipeTex_rt) return false;

	SDL_GPUBufferCreateInfo bi2 = {};
	bi2.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
	bi2.size  = kMaxVerts * sizeof(float) * 2;
	vbuf2D_rt = SDL_CreateGPUBuffer(device, &bi2);

	SDL_GPUBufferCreateInfo biT = {};
	biT.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
	biT.size  = kMaxTexVerts * sizeof(float) * 4;
	vbufTex_rt = SDL_CreateGPUBuffer(device, &biT);

	return vbuf2D_rt && vbufTex_rt;
}

bool Renderer::initPipelineBlur()
{
	if (!device || !window) return false;

	constexpr SDL_GPUTextureFormat kRTFmt = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;

	SDL_GPUShader* vert = loadShader(SDL_GPU_SHADERSTAGE_VERTEX,
	                                  "assets/shaders/rect2d_tex_vert.spv", 0, 0);
	SDL_GPUShader* frag = loadShader(SDL_GPU_SHADERSTAGE_FRAGMENT,
	                                  "assets/shaders/blur_frag.spv", 1, 1);
	if (!vert || !frag) {
		if (vert) SDL_ReleaseGPUShader(device, vert);
		if (frag) SDL_ReleaseGPUShader(device, frag);
		return false;
	}

	pipeBlur = makePipelineTex(kRTFmt, vert, frag);
	SDL_ReleaseGPUShader(device, vert);
	SDL_ReleaseGPUShader(device, frag);
	return pipeBlur != nullptr;
}

bool Renderer::initPipeline3D()
{
	if (!device || !window) return false;

	SDL_GPUShader* vert = loadShader(SDL_GPU_SHADERSTAGE_VERTEX,
	                                  "assets/shaders/mesh3d_vert.spv", 0, 1);
	SDL_GPUShader* frag = loadShader(SDL_GPU_SHADERSTAGE_FRAGMENT,
	                                  "assets/shaders/mesh3d_frag.spv", 1, 1);
	if (!vert || !frag) {
		if (vert) SDL_ReleaseGPUShader(device, vert);
		if (frag) SDL_ReleaseGPUShader(device, frag);
		std::cerr << "[Renderer] 3D pipeline unavailable (mesh3d shaders missing).\n";
		return false;
	}

	pipe3D = makePipeline3D(swapFmt, vert, frag);
	SDL_ReleaseGPUShader(device, vert);
	SDL_ReleaseGPUShader(device, frag);
	if (!pipe3D) { std::cerr << "[Renderer] pipe3D failed.\n"; return false; }
	return true;
}

// ============================================================================
// Depth buffer
// ============================================================================

void Renderer::ensureDepthBuffer(Uint32 w, Uint32 h)
{
	if (depthTex && depthW == w && depthH == h) return;
	if (depthTex) { SDL_ReleaseGPUTexture(device, depthTex); depthTex = nullptr; }

	SDL_GPUTextureCreateInfo tci = {};
	tci.type                     = SDL_GPU_TEXTURETYPE_2D;
	tci.format                   = SDL_GPU_TEXTUREFORMAT_D16_UNORM;
	tci.usage                    = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
	tci.width                    = w;
	tci.height                   = h;
	tci.layer_count_or_depth     = 1;
	tci.num_levels               = 1;
	depthTex = SDL_CreateGPUTexture(device, &tci);
	depthW   = w;
	depthH   = h;
}

// ============================================================================
// Vertex helpers
// ============================================================================

void Renderer::addQuad(std::vector<float>& v, float x1, float y1, float x2, float y2)
{
	v.insert(v.end(), {
		x1, y1,  x2, y1,  x1, y2,
		x2, y1,  x2, y2,  x1, y2
	});
}

void Renderer::addTexQuad(std::vector<float>& v,
                           float x1, float y1, float x2, float y2,
                           float u0, float v0, float u1, float v1)
{
	v.insert(v.end(), {
		x1, y1, u0, v0,
		x2, y1, u1, v0,
		x1, y2, u0, v1,
		x2, y1, u1, v0,
		x2, y2, u1, v1,
		x1, y2, u0, v1
	});
}

// ============================================================================
// Core flush — render a draw queue to any GPU texture
// ============================================================================

void Renderer::flushQueueTo(SDL_GPUCommandBuffer* cmd,
                             const std::vector<DrawCmd>& q,
                             const FlushParams& p)
{
	struct ColoredRange { Uint32 first, count; float r, g, b, a; };
	struct TexRange     { Uint32 first, count; float r, g, b, a; Texture* tex; };
	struct DrawRange {
		bool   textured;
		Uint32 first;
		Uint32 count;
		float  r, g, b, a;
		Texture* tex;
	};

	std::vector<float>        cv;
	std::vector<ColoredRange> coloredRanges;
	std::vector<float>        tv;
	std::vector<TexRange>     texRanges;
	std::vector<DrawRange>    drawRanges;

	const float tw = (float)p.tw;
	const float th = (float)p.th;

	for (const auto& c : q) {
		float ox = c.ss ? 0.f : camX;
		float oy = c.ss ? 0.f : camY;

		// NDC corners before any rotation
		float x1 =  2.f * (c.x       - ox) / tw - 1.f;
		float y1 =  1.f - 2.f * (c.y       - oy) / th;
		float x2 =  2.f * (c.x + c.w - ox) / tw - 1.f;
		float y2 =  1.f - 2.f * (c.y + c.h - oy) / th;

		// ── Filled circle — triangle fan ──────────────────────────────────
		if (c.isCircle) {
			float ncx = 2.f * (c.pivX - ox) / tw - 1.f;
			float ncy = 1.f - 2.f * (c.pivY - oy) / th;
			float nrx = 2.f * (c.w * 0.5f) / tw;
			float nry = 2.f * (c.h * 0.5f) / th;
			int segs  = c.segs > 4 ? c.segs : 32;
			Uint32 first = (Uint32)(cv.size() / 2);
			for (int i = 0; i < segs; ++i) {
				float a0 = 2.f * kPI * i       / segs;
				float a1 = 2.f * kPI * (i + 1) / segs;
				cv.insert(cv.end(), {
					ncx, ncy,
					ncx + nrx * cosf(a0), ncy - nry * sinf(a0),
					ncx + nrx * cosf(a1), ncy - nry * sinf(a1)
				});
			}
			Uint32 count = (Uint32)(cv.size() / 2) - first;
			coloredRanges.push_back({ first, count, c.r, c.g, c.b, c.a });
			drawRanges.push_back({ false, first, count, c.r, c.g, c.b, c.a, nullptr });
			continue;
		}

		// ── Rotated draw ──────────────────────────────────────────────────
		if (c.angle != 0.f) {
			float pnx  =  2.f * (c.pivX - ox) / tw - 1.f;
			float pny  =  1.f - 2.f * (c.pivY - oy) / th;
			float cosA = cosf(c.angle * kPI / 180.f);
			float sinA = sinf(c.angle * kPI / 180.f);

			auto rot = [&](float nx, float ny, float& rx, float& ry) {
				float dx = nx - pnx, dy = ny - pny;
				rx = pnx + dx * cosA - dy * sinA;
				ry = pny + dx * sinA + dy * cosA;
			};

			float ax, ay, bx, by, cx2, cy2, dx2, dy2;
			rot(x1, y1, ax, ay);
			rot(x2, y1, bx, by);
			rot(x1, y2, cx2, cy2);
			rot(x2, y2, dx2, dy2);

			if (c.texture) {
				Uint32 first = (Uint32)(tv.size() / 4);
				tv.insert(tv.end(), {
					ax,  ay,  c.u0, c.v0,
					bx,  by,  c.u1, c.v0,
					cx2, cy2, c.u0, c.v1,
					bx,  by,  c.u1, c.v0,
					dx2, dy2, c.u1, c.v1,
					cx2, cy2, c.u0, c.v1
				});
				Uint32 count = (Uint32)(tv.size() / 4) - first;
				texRanges.push_back({ first, count, c.r, c.g, c.b, c.a, c.texture });
				drawRanges.push_back({ true, first, count, c.r, c.g, c.b, c.a, c.texture });
			} else {
				Uint32 first = (Uint32)(cv.size() / 2);
				cv.insert(cv.end(), {
					ax,  ay,  bx,  by,  cx2, cy2,
					bx,  by,  dx2, dy2, cx2, cy2
				});
				Uint32 count = (Uint32)(cv.size() / 2) - first;
				coloredRanges.push_back({ first, count, c.r, c.g, c.b, c.a });
				drawRanges.push_back({ false, first, count, c.r, c.g, c.b, c.a, nullptr });
			}
			continue;
		}

		// ── Standard (axis-aligned) draw ──────────────────────────────────
		if (c.texture) {
			Uint32 first = (Uint32)(tv.size() / 4);
			addTexQuad(tv, x1, y1, x2, y2, c.u0, c.v0, c.u1, c.v1);
			Uint32 count = (Uint32)(tv.size() / 4) - first;
			texRanges.push_back({ first, count, c.r, c.g, c.b, c.a, c.texture });
			drawRanges.push_back({ true, first, count, c.r, c.g, c.b, c.a, c.texture });
		} else {
			Uint32 first = (Uint32)(cv.size() / 2);
			if (c.fill) {
				addQuad(cv, x1, y1, x2, y2);
			} else {
				float px = 2.f / tw;
				float py = 2.f / th;
				addQuad(cv, x1,      y1,      x2,      y1 - py);
				addQuad(cv, x1,      y2 + py, x2,      y2);
				addQuad(cv, x1,      y1,      x1 + px, y2);
				addQuad(cv, x2 - px, y1,      x2,      y2);
			}
			Uint32 count = (Uint32)(cv.size() / 2) - first;
			coloredRanges.push_back({ first, count, c.r, c.g, c.b, c.a });
			drawRanges.push_back({ false, first, count, c.r, c.g, c.b, c.a, nullptr });
		}
	}

	// ── Copy pass: upload vertex data ──
	if (!cv.empty() || !tv.empty()) {
		SDL_GPUCopyPass* cp = SDL_BeginGPUCopyPass(cmd);

		if (p.p2D && !cv.empty()) {
			Uint32 vSize = (Uint32)(cv.size() * sizeof(float));
			SDL_GPUTransferBufferCreateInfo tbi = { SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, vSize };
			SDL_GPUTransferBuffer* tb = SDL_CreateGPUTransferBuffer(device, &tbi);
			if (tb) {
				SDL_memcpy(SDL_MapGPUTransferBuffer(device, tb, false), cv.data(), vSize);
				SDL_UnmapGPUTransferBuffer(device, tb);
				SDL_GPUTransferBufferLocation src = { tb, 0 };
				SDL_GPUBufferRegion           dst = { p.vb2D, 0, vSize };
				SDL_UploadToGPUBuffer(cp, &src, &dst, true);
				SDL_ReleaseGPUTransferBuffer(device, tb);
			}
		}

		if (p.pTex && !tv.empty()) {
			Uint32 vSize = (Uint32)(tv.size() * sizeof(float));
			SDL_GPUTransferBufferCreateInfo tbi = { SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, vSize };
			SDL_GPUTransferBuffer* tb = SDL_CreateGPUTransferBuffer(device, &tbi);
			if (tb) {
				SDL_memcpy(SDL_MapGPUTransferBuffer(device, tb, false), tv.data(), vSize);
				SDL_UnmapGPUTransferBuffer(device, tb);
				SDL_GPUTransferBufferLocation src = { tb, 0 };
				SDL_GPUBufferRegion           dst = { p.vbTex, 0, vSize };
				SDL_UploadToGPUBuffer(cp, &src, &dst, true);
				SDL_ReleaseGPUTransferBuffer(device, tb);
			}
		}

		SDL_EndGPUCopyPass(cp);
	}

	// ── Render pass ──
	SDL_GPUColorTargetInfo ct = {};
	ct.texture     = p.target;
	ct.clear_color = p.clearColor;
	ct.load_op     = p.clear ? SDL_GPU_LOADOP_CLEAR : SDL_GPU_LOADOP_LOAD;
	ct.store_op    = SDL_GPU_STOREOP_STORE;

	SDL_GPURenderPass* rp = SDL_BeginGPURenderPass(cmd, &ct, 1, nullptr);

	bool hasBound = false;
	bool boundTextured = false;
	for (const auto& r : drawRanges) {
		if (r.textured) {
			if (!p.pTex || !r.tex) continue;
			if (!hasBound || !boundTextured) {
				SDL_BindGPUGraphicsPipeline(rp, p.pTex);
				SDL_GPUBufferBinding vb = { p.vbTex, 0 };
				SDL_BindGPUVertexBuffers(rp, 0, &vb, 1);
				hasBound = true;
				boundTextured = true;
			}
			SDL_GPUTextureSamplerBinding tsb = { r.tex->GetGPUTexture(), r.tex->GetSampler() };
			SDL_BindGPUFragmentSamplers(rp, 0, &tsb, 1);
			float col[4] = { r.r, r.g, r.b, r.a };
			SDL_PushGPUFragmentUniformData(cmd, 0, col, sizeof(col));
			SDL_DrawGPUPrimitives(rp, r.count, 1, r.first, 0);
		} else {
			if (!p.p2D) continue;
			if (!hasBound || boundTextured) {
				SDL_BindGPUGraphicsPipeline(rp, p.p2D);
				SDL_GPUBufferBinding vb = { p.vb2D, 0 };
				SDL_BindGPUVertexBuffers(rp, 0, &vb, 1);
				hasBound = true;
				boundTextured = false;
			}
			float col[4] = { r.r, r.g, r.b, r.a };
			SDL_PushGPUFragmentUniformData(cmd, 0, col, sizeof(col));
			SDL_DrawGPUPrimitives(rp, r.count, 1, r.first, 0);
		}
	}

	SDL_EndGPURenderPass(rp);
}

// ============================================================================
// 3D render pass
// ============================================================================

void Renderer::render3DPass(SDL_GPUCommandBuffer* cmd,
                             SDL_GPUTexture* swapTex, Uint32 sw, Uint32 sh)
{
	if (mesh3DQueue.empty() || !pipe3D) return;

	ensureDepthBuffer(sw, sh);

	SDL_GPUColorTargetInfo ct = {};
	ct.texture     = swapTex;
	ct.clear_color = clearColor;
	ct.load_op     = SDL_GPU_LOADOP_CLEAR;
	ct.store_op    = SDL_GPU_STOREOP_STORE;

	SDL_GPUDepthStencilTargetInfo dt = {};
	dt.texture            = depthTex;
	dt.clear_depth        = 1.0f;
	dt.load_op            = SDL_GPU_LOADOP_CLEAR;
	dt.store_op           = SDL_GPU_STOREOP_DONT_CARE;
	dt.stencil_load_op    = SDL_GPU_LOADOP_DONT_CARE;
	dt.stencil_store_op   = SDL_GPU_STOREOP_DONT_CARE;
	dt.cycle              = true;

	SDL_GPURenderPass* rp = SDL_BeginGPURenderPass(cmd, &ct, 1, &dt);
	SDL_BindGPUGraphicsPipeline(rp, pipe3D);

	// Set viewport
	SDL_GPUViewport vp = {};
	vp.x = 0; vp.y = 0; vp.w = (float)sw; vp.h = (float)sh;
	vp.min_depth = 0.f; vp.max_depth = 1.f;
	SDL_SetGPUViewport(rp, &vp);

	for (const auto& mc : mesh3DQueue) {
		if (!mc.mesh || !mc.mesh->IsValid()) continue;

		SDL_GPUBufferBinding vb = { mc.mesh->vbuf, 0 };
		SDL_BindGPUVertexBuffers(rp, 0, &vb, 1);
		SDL_GPUBufferBinding ib = { mc.mesh->ibuf, 0 };
		SDL_BindGPUIndexBuffer(rp, &ib, SDL_GPU_INDEXELEMENTSIZE_32BIT);

		// Vertex uniform slot 0: mat4 mvp + mat4 model = 128 bytes
		struct { glm::mat4 mvp; glm::mat4 model; } vu;
		vu.mvp   = projMat * viewMat * mc.model;
		vu.model = mc.model;
		SDL_PushGPUVertexUniformData(cmd, 0, &vu, sizeof(vu));

		// Fragment sampler slot 0: albedo
		Texture* albedo = (mc.albedo && mc.albedo->IsValid()) ? mc.albedo : &whiteTex;
		SDL_GPUTextureSamplerBinding tsb = { albedo->GetGPUTexture(), albedo->GetSampler() };
		SDL_BindGPUFragmentSamplers(rp, 0, &tsb, 1);

		// Fragment uniform slot 0: light params (64 bytes)
		SDL_PushGPUFragmentUniformData(cmd, 0, &sceneLight, sizeof(sceneLight));

		SDL_DrawGPUIndexedPrimitives(rp, mc.mesh->IndexCount(), 1, 0, 0, 0);
	}

	SDL_EndGPURenderPass(rp);
	mesh3DQueue.clear();
}

// ============================================================================
// Public API
// ============================================================================

void Renderer::Release()
{
	if (device) {
		whiteTex.Release();
		if (depthTex)   { SDL_ReleaseGPUTexture(device, depthTex);           depthTex   = nullptr; }
		if (pipe3D)     { SDL_ReleaseGPUGraphicsPipeline(device, pipe3D);    pipe3D     = nullptr; }
		if (pipeBlur)   { SDL_ReleaseGPUGraphicsPipeline(device, pipeBlur);  pipeBlur   = nullptr; }
		if (vbufTex_rt) { SDL_ReleaseGPUBuffer(device, vbufTex_rt);          vbufTex_rt = nullptr; }
		if (vbuf2D_rt)  { SDL_ReleaseGPUBuffer(device, vbuf2D_rt);           vbuf2D_rt  = nullptr; }
		if (pipeTex_rt) { SDL_ReleaseGPUGraphicsPipeline(device, pipeTex_rt);pipeTex_rt = nullptr; }
		if (pipe2D_rt)  { SDL_ReleaseGPUGraphicsPipeline(device, pipe2D_rt); pipe2D_rt  = nullptr; }
		if (vbufTex)    { SDL_ReleaseGPUBuffer(device, vbufTex);             vbufTex    = nullptr; }
		if (pipeTex)    { SDL_ReleaseGPUGraphicsPipeline(device, pipeTex);   pipeTex    = nullptr; }
		if (vbuf2D)     { SDL_ReleaseGPUBuffer(device, vbuf2D);              vbuf2D     = nullptr; }
		if (pipe2D)     { SDL_ReleaseGPUGraphicsPipeline(device, pipe2D);    pipe2D     = nullptr; }
	}
	device = nullptr;
	window = nullptr;
}

Texture Renderer::LoadTexture(const char* path)
{
	SDL_Surface* surf = SDL_LoadBMP(path);
	if (!surf) {
		std::cerr << "[Renderer] SDL_LoadBMP failed: " << SDL_GetError() << "\n";
		return Texture{};
	}
	SDL_Surface* rgba = SDL_ConvertSurface(surf, SDL_PIXELFORMAT_RGBA32);
	SDL_DestroySurface(surf);
	if (!rgba) {
		std::cerr << "[Renderer] SDL_ConvertSurface failed: " << SDL_GetError() << "\n";
		return Texture{};
	}
	Texture tex = Texture::FromPixels(device, rgba->pixels, rgba->w, rgba->h);
	SDL_DestroySurface(rgba);
	return tex;
}

Texture Renderer::LoadTextureFromPixels(const void* rgba8, int w, int h, bool nearestFilter)
{
	return Texture::FromPixels(device, rgba8, w, h, nearestFilter);
}

void Renderer::SetClearColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	clearColor = { r / 255.f, g / 255.f, b / 255.f, a / 255.f };
}

void Renderer::Clear()
{
	mainQueue.clear();
	mesh3DQueue.clear();
	fbQueue.clear();
	activeQueue = &mainQueue;
	fbTarget    = nullptr;
}

void Renderer::SetDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	dr = r / 255.f; dg = g / 255.f; db = b / 255.f; da = a / 255.f;
}

void Renderer::FillRect(float x, float y, float w, float h)
{
	if (!pipe2D) return;
	activeQueue->push_back({ x, y, w, h, dr, dg, db, da, true,
	                         nullptr, 0.f, 0.f, 1.f, 1.f, screenSpace });
}

void Renderer::DrawRect(float x, float y, float w, float h)
{
	if (!pipe2D) return;
	activeQueue->push_back({ x, y, w, h, dr, dg, db, da, false,
	                         nullptr, 0.f, 0.f, 1.f, 1.f, screenSpace });
}

void Renderer::DrawTexture(Texture& tex, float x, float y, float w, float h)
{
	if (!pipeTex || !tex.IsValid()) return;
	if (w == 0.f) w = (float)tex.GetWidth();
	if (h == 0.f) h = (float)tex.GetHeight();
	activeQueue->push_back({ x, y, w, h, dr, dg, db, da, true,
	                         &tex, 0.f, 0.f, 1.f, 1.f, screenSpace });
}

void Renderer::DrawTextureRegion(Texture& tex, float x, float y, float w, float h,
                                  float u0, float v0, float u1, float v1)
{
	if (!pipeTex || !tex.IsValid()) return;
	activeQueue->push_back({ x, y, w, h, dr, dg, db, da, true,
	                         &tex, u0, v0, u1, v1, screenSpace });
}

void Renderer::DrawTextureEx(Texture& tex, float x, float y, float w, float h,
                              float angle, float pivX, float pivY)
{
	if (!pipeTex || !tex.IsValid()) return;
	if (w == 0.f) w = (float)tex.GetWidth();
	if (h == 0.f) h = (float)tex.GetHeight();
	activeQueue->push_back({ x, y, w, h, dr, dg, db, da, true,
	                         &tex, 0.f, 0.f, 1.f, 1.f, screenSpace,
	                         angle, pivX, pivY, false, 0 });
}

void Renderer::DrawLine(float x1, float y1, float x2, float y2, float thickness)
{
	if (!pipe2D) return;
	float dx  = x2 - x1, dy = y2 - y1;
	float len = sqrtf(dx * dx + dy * dy);
	if (len < 0.5f) return;

	float angle = atan2f(dy, dx) * 180.f / kPI;
	float cx    = (x1 + x2) * 0.5f;
	float cy    = (y1 + y2) * 0.5f;
	// Rect top-left: centered at (cx,cy), width=len, height=thickness
	float rx = cx - len * 0.5f;
	float ry = cy - thickness * 0.5f;
	activeQueue->push_back({ rx, ry, len, thickness, dr, dg, db, da, true,
	                         nullptr, 0.f, 0.f, 1.f, 1.f, screenSpace,
	                         angle, cx, cy, false, 0 });
}

void Renderer::DrawCircle(float cx, float cy, float radius, int segs)
{
	if (segs < 8) segs = 32;
	for (int i = 0; i < segs; ++i) {
		float a0 = 2.f * kPI * i       / segs;
		float a1 = 2.f * kPI * (i + 1) / segs;
		DrawLine(cx + cosf(a0) * radius, cy + sinf(a0) * radius,
		         cx + cosf(a1) * radius, cy + sinf(a1) * radius, 1.f);
	}
}

void Renderer::FillCircle(float cx, float cy, float radius, int segs)
{
	if (!pipe2D) return;
	float d = radius * 2.f;
	// isCircle=true, pivot at (cx,cy) = actual circle centre
	activeQueue->push_back({ cx - radius, cy - radius, d, d,
	                         dr, dg, db, da, true,
	                         nullptr, 0.f, 0.f, 1.f, 1.f, screenSpace,
	                         0.f, cx, cy, true, segs });
}

// ── 3D API ───────────────────────────────────────────────────────────────────

void Renderer::UploadMesh(Mesh& mesh)
{
	if (mesh.vertices.empty() || mesh.indices.empty() || !device) return;

	// Release existing GPU buffers if any
	if (mesh.vbuf) { SDL_ReleaseGPUBuffer(device, mesh.vbuf); mesh.vbuf = nullptr; }
	if (mesh.ibuf) { SDL_ReleaseGPUBuffer(device, mesh.ibuf); mesh.ibuf = nullptr; }

	Uint32 vSize = (Uint32)(mesh.vertices.size() * sizeof(float));
	Uint32 iSize = (Uint32)(mesh.indices.size()  * sizeof(Uint32));

	SDL_GPUBufferCreateInfo vbi = {}; vbi.usage = SDL_GPU_BUFFERUSAGE_VERTEX; vbi.size = vSize;
	SDL_GPUBufferCreateInfo ibi = {}; ibi.usage = SDL_GPU_BUFFERUSAGE_INDEX;  ibi.size = iSize;
	mesh.vbuf = SDL_CreateGPUBuffer(device, &vbi);
	mesh.ibuf = SDL_CreateGPUBuffer(device, &ibi);
	if (!mesh.vbuf || !mesh.ibuf) return;

	SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);
	if (!cmd) return;

	SDL_GPUTransferBufferCreateInfo tbi = { SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, vSize + iSize };
	SDL_GPUTransferBuffer* tb = SDL_CreateGPUTransferBuffer(device, &tbi);
	if (!tb) { SDL_SubmitGPUCommandBuffer(cmd); return; }

	Uint8* map = (Uint8*)SDL_MapGPUTransferBuffer(device, tb, false);
	SDL_memcpy(map,         mesh.vertices.data(), vSize);
	SDL_memcpy(map + vSize, mesh.indices.data(),  iSize);
	SDL_UnmapGPUTransferBuffer(device, tb);

	SDL_GPUCopyPass* cp = SDL_BeginGPUCopyPass(cmd);
	SDL_GPUTransferBufferLocation src0 = { tb, 0       }; SDL_GPUBufferRegion dst0 = { mesh.vbuf, 0, vSize };
	SDL_GPUTransferBufferLocation src1 = { tb, vSize   }; SDL_GPUBufferRegion dst1 = { mesh.ibuf, 0, iSize };
	SDL_UploadToGPUBuffer(cp, &src0, &dst0, false);
	SDL_UploadToGPUBuffer(cp, &src1, &dst1, false);
	SDL_EndGPUCopyPass(cp);
	SDL_ReleaseGPUTransferBuffer(device, tb);
	SDL_SubmitGPUCommandBuffer(cmd);
	SDL_WaitForGPUIdle(device);
}

void Renderer::ReleaseMesh(Mesh& mesh)
{
	if (!device) return;
	if (mesh.vbuf) { SDL_ReleaseGPUBuffer(device, mesh.vbuf); mesh.vbuf = nullptr; }
	if (mesh.ibuf) { SDL_ReleaseGPUBuffer(device, mesh.ibuf); mesh.ibuf = nullptr; }
}

void Renderer::DrawMesh(Mesh& mesh, const glm::mat4& model, Texture* albedo)
{
	if (!pipe3D || !mesh.IsValid()) return;
	mesh3DQueue.push_back({ &mesh, model, albedo });
}

// ── Framebuffer API ──────────────────────────────────────────────────────────

void Renderer::BeginRenderToTexture(Framebuffer& fb)
{
	if (!fb.IsValid()) return;
	fbTarget  = fb.GetGPUTexture();
	fbTargetW = fb.GetWidth();
	fbTargetH = fb.GetHeight();
	fbQueue.clear();
	activeQueue = &fbQueue;
}

void Renderer::EndRenderToTexture()
{
	if (!fbTarget || fbQueue.empty()) {
		activeQueue = &mainQueue;
		fbTarget    = nullptr;
		return;
	}

	SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);
	if (!cmd) { activeQueue = &mainQueue; fbTarget = nullptr; return; }

	FlushParams p;
	p.target     = fbTarget;
	p.tw         = (Uint32)fbTargetW;
	p.th         = (Uint32)fbTargetH;
	p.vb2D       = vbuf2D_rt;
	p.vbTex      = vbufTex_rt;
	p.p2D        = pipe2D_rt;
	p.pTex       = pipeTex_rt;
	p.clear      = true;
	p.clearColor = { 0.f, 0.f, 0.f, 0.f };

	flushQueueTo(cmd, fbQueue, p);
	SDL_SubmitGPUCommandBuffer(cmd);
	SDL_WaitForGPUIdle(device);

	fbQueue.clear();
	activeQueue = &mainQueue;
	fbTarget    = nullptr;
}

void Renderer::BlurTexture(Framebuffer& dest, Texture& src, float radius)
{
	if (!pipeBlur || !dest.IsValid() || !src.IsValid()) return;

	SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);
	if (!cmd) return;

	std::vector<float> tv;
	addTexQuad(tv, -1.f, -1.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f);

	Uint32 vSize = (Uint32)(tv.size() * sizeof(float));
	SDL_GPUTransferBufferCreateInfo tbi = { SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, vSize };
	SDL_GPUTransferBuffer* tb = SDL_CreateGPUTransferBuffer(device, &tbi);
	if (!tb) { SDL_SubmitGPUCommandBuffer(cmd); return; }

	SDL_memcpy(SDL_MapGPUTransferBuffer(device, tb, false), tv.data(), vSize);
	SDL_UnmapGPUTransferBuffer(device, tb);

	SDL_GPUCopyPass* cp = SDL_BeginGPUCopyPass(cmd);
	SDL_GPUTransferBufferLocation src_loc = { tb, 0 };
	SDL_GPUBufferRegion           dst_reg = { vbufTex_rt, 0, vSize };
	SDL_UploadToGPUBuffer(cp, &src_loc, &dst_reg, true);
	SDL_EndGPUCopyPass(cp);
	SDL_ReleaseGPUTransferBuffer(device, tb);

	SDL_GPUColorTargetInfo ct = {};
	ct.texture     = dest.GetGPUTexture();
	ct.clear_color = { 0.f, 0.f, 0.f, 0.f };
	ct.load_op     = SDL_GPU_LOADOP_CLEAR;
	ct.store_op    = SDL_GPU_STOREOP_STORE;

	SDL_GPURenderPass* rp = SDL_BeginGPURenderPass(cmd, &ct, 1, nullptr);
	SDL_BindGPUGraphicsPipeline(rp, pipeBlur);

	SDL_GPUBufferBinding vb = { vbufTex_rt, 0 };
	SDL_BindGPUVertexBuffers(rp, 0, &vb, 1);

	SDL_GPUTextureSamplerBinding tsb = { src.GetGPUTexture(), src.GetSampler() };
	SDL_BindGPUFragmentSamplers(rp, 0, &tsb, 1);

	float params[4] = { radius, 0.f, (float)src.GetWidth(), (float)src.GetHeight() };
	SDL_PushGPUFragmentUniformData(cmd, 0, params, sizeof(params));

	SDL_DrawGPUPrimitives(rp, 6, 1, 0, 0);
	SDL_EndGPURenderPass(rp);
	SDL_SubmitGPUCommandBuffer(cmd);
	SDL_WaitForGPUIdle(device);
}

// ── Present ─────────────────────────────────────────────────────────────────

void Renderer::Present()
{
	if (!device || !window) return;

	SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);
	if (!cmd) return;

	SDL_GPUTexture* swapTex = nullptr;
	Uint32 sw = 0, sh = 0;
	if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmd, window, &swapTex, &sw, &sh) || !swapTex) {
		SDL_SubmitGPUCommandBuffer(cmd);
		mainQueue.clear();
		mesh3DQueue.clear();
		return;
	}

	bool has3D = !mesh3DQueue.empty() && pipe3D;

	// 3D pass first — clears swapchain + depth, renders meshes
	if (has3D) {
		render3DPass(cmd, swapTex, sw, sh);
	}

	// 2D pass — clear only if no 3D output to preserve
	FlushParams p;
	p.target     = swapTex;
	p.tw         = sw;
	p.th         = sh;
	p.vb2D       = vbuf2D;
	p.vbTex      = vbufTex;
	p.p2D        = pipe2D;
	p.pTex       = pipeTex;
	p.clear      = !has3D;
	p.clearColor = clearColor;

	flushQueueTo(cmd, mainQueue, p);
	SDL_SubmitGPUCommandBuffer(cmd);
	mainQueue.clear();
}

} // namespace LightningEngine
