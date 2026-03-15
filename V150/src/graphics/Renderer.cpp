#include "../include/Renderer.h"
#include <iostream>

namespace LightningEngine {

	Renderer::Renderer(SDL_GPUDevice* dev, SDL_Window* win)
		: device(dev), window(win)
	{
		initPipeline2D();
		initPipelineTex();
	}

	// -----------------------------------------------------------------------
	// Internal helpers
	// -----------------------------------------------------------------------

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

	bool Renderer::initPipeline2D()
	{
		if (!device || !window) return false;

		SDL_GPUShader* vert = loadShader(SDL_GPU_SHADERSTAGE_VERTEX,   "assets/shaders/rect2d_vert.spv", 0, 0);
		SDL_GPUShader* frag = loadShader(SDL_GPU_SHADERSTAGE_FRAGMENT, "assets/shaders/rect2d_frag.spv", 0, 1);

		if (!vert || !frag) {
			if (vert) SDL_ReleaseGPUShader(device, vert);
			if (frag) SDL_ReleaseGPUShader(device, frag);
			std::cerr << "[Renderer] Colored 2D pipeline unavailable — run compile_shaders.bat first.\n";
			return false;
		}

		SDL_GPUVertexBufferDescription vbDesc = {};
		vbDesc.slot       = 0;
		vbDesc.pitch      = sizeof(float) * 2; // vec2 pos
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
		colorTarget.format     = SDL_GetGPUSwapchainTextureFormat(device, window);
		colorTarget.blend_state = blend;

		SDL_GPUGraphicsPipelineCreateInfo pipeInfo = {};
		pipeInfo.vertex_shader                                 = vert;
		pipeInfo.fragment_shader                               = frag;
		pipeInfo.vertex_input_state.vertex_buffer_descriptions = &vbDesc;
		pipeInfo.vertex_input_state.num_vertex_buffers         = 1;
		pipeInfo.vertex_input_state.vertex_attributes          = &attr;
		pipeInfo.vertex_input_state.num_vertex_attributes      = 1;
		pipeInfo.primitive_type                                = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
		pipeInfo.rasterizer_state.fill_mode                    = SDL_GPU_FILLMODE_FILL;
		pipeInfo.rasterizer_state.cull_mode                    = SDL_GPU_CULLMODE_NONE;
		pipeInfo.target_info.color_target_descriptions         = &colorTarget;
		pipeInfo.target_info.num_color_targets                 = 1;

		pipe2D = SDL_CreateGPUGraphicsPipeline(device, &pipeInfo);

		SDL_ReleaseGPUShader(device, vert);
		SDL_ReleaseGPUShader(device, frag);

		if (!pipe2D) {
			std::cerr << "[Renderer] SDL_CreateGPUGraphicsPipeline (2D) failed: " << SDL_GetError() << "\n";
			return false;
		}

		SDL_GPUBufferCreateInfo bufInfo = {};
		bufInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
		bufInfo.size  = kMaxVerts * sizeof(float) * 2;

		vbuf2D = SDL_CreateGPUBuffer(device, &bufInfo);
		if (!vbuf2D) {
			std::cerr << "[Renderer] SDL_CreateGPUBuffer (2D) failed: " << SDL_GetError() << "\n";
			SDL_ReleaseGPUGraphicsPipeline(device, pipe2D);
			pipe2D = nullptr;
			return false;
		}

		return true;
	}

	bool Renderer::initPipelineTex()
	{
		if (!device || !window) return false;

		SDL_GPUShader* vert = loadShader(SDL_GPU_SHADERSTAGE_VERTEX,   "assets/shaders/rect2d_tex_vert.spv", 0, 0);
		SDL_GPUShader* frag = loadShader(SDL_GPU_SHADERSTAGE_FRAGMENT, "assets/shaders/rect2d_tex_frag.spv", 1, 1);

		if (!vert || !frag) {
			if (vert) SDL_ReleaseGPUShader(device, vert);
			if (frag) SDL_ReleaseGPUShader(device, frag);
			std::cerr << "[Renderer] Textured 2D pipeline unavailable — run compile_shaders.bat first.\n";
			return false;
		}

		// Interleaved: vec2 pos + vec2 uv = 16 bytes per vertex
		SDL_GPUVertexBufferDescription vbDesc = {};
		vbDesc.slot       = 0;
		vbDesc.pitch      = sizeof(float) * 4;
		vbDesc.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;

		SDL_GPUVertexAttribute attrs[2] = {};
		attrs[0].location    = 0;
		attrs[0].buffer_slot = 0;
		attrs[0].format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
		attrs[0].offset      = 0;
		attrs[1].location    = 1;
		attrs[1].buffer_slot = 0;
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
		colorTarget.format      = SDL_GetGPUSwapchainTextureFormat(device, window);
		colorTarget.blend_state = blend;

		SDL_GPUGraphicsPipelineCreateInfo pipeInfo = {};
		pipeInfo.vertex_shader                                 = vert;
		pipeInfo.fragment_shader                               = frag;
		pipeInfo.vertex_input_state.vertex_buffer_descriptions = &vbDesc;
		pipeInfo.vertex_input_state.num_vertex_buffers         = 1;
		pipeInfo.vertex_input_state.vertex_attributes          = attrs;
		pipeInfo.vertex_input_state.num_vertex_attributes      = 2;
		pipeInfo.primitive_type                                = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
		pipeInfo.rasterizer_state.fill_mode                    = SDL_GPU_FILLMODE_FILL;
		pipeInfo.rasterizer_state.cull_mode                    = SDL_GPU_CULLMODE_NONE;
		pipeInfo.target_info.color_target_descriptions         = &colorTarget;
		pipeInfo.target_info.num_color_targets                 = 1;

		pipeTex = SDL_CreateGPUGraphicsPipeline(device, &pipeInfo);

		SDL_ReleaseGPUShader(device, vert);
		SDL_ReleaseGPUShader(device, frag);

		if (!pipeTex) {
			std::cerr << "[Renderer] SDL_CreateGPUGraphicsPipeline (Tex) failed: " << SDL_GetError() << "\n";
			return false;
		}

		SDL_GPUBufferCreateInfo bufInfo = {};
		bufInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
		bufInfo.size  = kMaxTexVerts * sizeof(float) * 4;

		vbufTex = SDL_CreateGPUBuffer(device, &bufInfo);
		if (!vbufTex) {
			std::cerr << "[Renderer] SDL_CreateGPUBuffer (Tex) failed: " << SDL_GetError() << "\n";
			SDL_ReleaseGPUGraphicsPipeline(device, pipeTex);
			pipeTex = nullptr;
			return false;
		}

		return true;
	}

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
		// 6 vertices, interleaved pos+uv
		v.insert(v.end(), {
			x1, y1, u0, v0,
			x2, y1, u1, v0,
			x1, y2, u0, v1,
			x2, y1, u1, v0,
			x2, y2, u1, v1,
			x1, y2, u0, v1
		});
	}

	// -----------------------------------------------------------------------
	// Public API
	// -----------------------------------------------------------------------

	void Renderer::Release()
	{
		if (device) {
			if (vbufTex) { SDL_ReleaseGPUBuffer(device, vbufTex);              vbufTex = nullptr; }
			if (pipeTex) { SDL_ReleaseGPUGraphicsPipeline(device, pipeTex);    pipeTex = nullptr; }
			if (vbuf2D)  { SDL_ReleaseGPUBuffer(device, vbuf2D);               vbuf2D  = nullptr; }
			if (pipe2D)  { SDL_ReleaseGPUGraphicsPipeline(device, pipe2D);     pipe2D  = nullptr; }
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

		// Convert to RGBA32 — bytes in memory will be [R, G, B, A] on all platforms
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

	void Renderer::SetClearColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
	{
		clearColor = { r / 255.f, g / 255.f, b / 255.f, a / 255.f };
	}

	void Renderer::Clear()
	{
		queue.clear();
	}

	void Renderer::SetDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
	{
		dr = r / 255.f; dg = g / 255.f; db = b / 255.f; da = a / 255.f;
	}

	void Renderer::FillRect(float x, float y, float w, float h)
	{
		if (!pipe2D) return;
		queue.push_back({ x, y, w, h, dr, dg, db, da, true, nullptr, 0.f, 0.f, 1.f, 1.f });
	}

	void Renderer::DrawRect(float x, float y, float w, float h)
	{
		if (!pipe2D) return;
		queue.push_back({ x, y, w, h, dr, dg, db, da, false, nullptr, 0.f, 0.f, 1.f, 1.f });
	}

	void Renderer::DrawTexture(Texture& tex, float x, float y, float w, float h)
	{
		if (!pipeTex || !tex.IsValid()) return;
		if (w == 0.f) w = (float)tex.GetWidth();
		if (h == 0.f) h = (float)tex.GetHeight();
		queue.push_back({ x, y, w, h, dr, dg, db, da, true, &tex, 0.f, 0.f, 1.f, 1.f });
	}

	void Renderer::DrawTextureRegion(Texture& tex, float x, float y, float w, float h,
	                                  float u0, float v0, float u1, float v1)
	{
		if (!pipeTex || !tex.IsValid()) return;
		queue.push_back({ x, y, w, h, dr, dg, db, da, true, &tex, u0, v0, u1, v1 });
	}

	void Renderer::Present()
	{
		if (!device || !window) return;

		SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);
		if (!cmd) return;

		SDL_GPUTexture* swapTex = nullptr;
		Uint32 sw = 0, sh = 0;
		if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmd, window, &swapTex, &sw, &sh) || !swapTex) {
			SDL_SubmitGPUCommandBuffer(cmd);
			queue.clear();
			return;
		}

		// --- Build vertex data — colored and textured are uploaded to separate buffers ---

		struct ColoredRange { Uint32 first, count; float r, g, b, a; };
		struct TexRange     { Uint32 first, count; float r, g, b, a; Texture* tex; };

		std::vector<float>        cv; // colored verts: vec2 pos
		std::vector<ColoredRange> coloredRanges;
		std::vector<float>        tv; // textured verts: vec2 pos + vec2 uv
		std::vector<TexRange>     texRanges;

		for (const auto& c : queue) {
			float x1 =  2.f * c.x         / (float)sw - 1.f;
			float y1 =  1.f - 2.f * c.y         / (float)sh;
			float x2 =  2.f * (c.x + c.w) / (float)sw - 1.f;
			float y2 =  1.f - 2.f * (c.y + c.h) / (float)sh;

			if (c.texture) {
				Uint32 first = (Uint32)(tv.size() / 4);
				addTexQuad(tv, x1, y1, x2, y2, c.u0, c.v0, c.u1, c.v1);
				Uint32 count = (Uint32)(tv.size() / 4) - first;
				texRanges.push_back({ first, count, c.r, c.g, c.b, c.a, c.texture });
			} else {
				Uint32 first = (Uint32)(cv.size() / 2);
				if (c.fill) {
					addQuad(cv, x1, y1, x2, y2);
				} else {
					float px = 2.f / (float)sw;
					float py = 2.f / (float)sh;
					addQuad(cv, x1,      y1,      x2,      y1 - py);
					addQuad(cv, x1,      y2 + py, x2,      y2);
					addQuad(cv, x1,      y1,      x1 + px, y2);
					addQuad(cv, x2 - px, y1,      x2,      y2);
				}
				Uint32 count = (Uint32)(cv.size() / 2) - first;
				coloredRanges.push_back({ first, count, c.r, c.g, c.b, c.a });
			}
		}

		// --- Copy pass: upload both vertex buffers ---
		if (!cv.empty() || !tv.empty()) {
			SDL_GPUCopyPass* cp = SDL_BeginGPUCopyPass(cmd);

			if (pipe2D && !cv.empty()) {
				Uint32 vSize = (Uint32)(cv.size() * sizeof(float));
				SDL_GPUTransferBufferCreateInfo tbInfo = { SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, vSize };
				SDL_GPUTransferBuffer* tb = SDL_CreateGPUTransferBuffer(device, &tbInfo);
				if (tb) {
					SDL_memcpy(SDL_MapGPUTransferBuffer(device, tb, false), cv.data(), vSize);
					SDL_UnmapGPUTransferBuffer(device, tb);
					SDL_GPUTransferBufferLocation src = { tb, 0 };
					SDL_GPUBufferRegion           dst = { vbuf2D, 0, vSize };
					SDL_UploadToGPUBuffer(cp, &src, &dst, true);
					SDL_ReleaseGPUTransferBuffer(device, tb);
				}
			}

			if (pipeTex && !tv.empty()) {
				Uint32 vSize = (Uint32)(tv.size() * sizeof(float));
				SDL_GPUTransferBufferCreateInfo tbInfo = { SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, vSize };
				SDL_GPUTransferBuffer* tb = SDL_CreateGPUTransferBuffer(device, &tbInfo);
				if (tb) {
					SDL_memcpy(SDL_MapGPUTransferBuffer(device, tb, false), tv.data(), vSize);
					SDL_UnmapGPUTransferBuffer(device, tb);
					SDL_GPUTransferBufferLocation src = { tb, 0 };
					SDL_GPUBufferRegion           dst = { vbufTex, 0, vSize };
					SDL_UploadToGPUBuffer(cp, &src, &dst, true);
					SDL_ReleaseGPUTransferBuffer(device, tb);
				}
			}

			SDL_EndGPUCopyPass(cp);
		}

		// --- Render pass ---
		SDL_GPUColorTargetInfo ct = {};
		ct.texture     = swapTex;
		ct.clear_color = clearColor;
		ct.load_op     = SDL_GPU_LOADOP_CLEAR;
		ct.store_op    = SDL_GPU_STOREOP_STORE;

		SDL_GPURenderPass* rp = SDL_BeginGPURenderPass(cmd, &ct, 1, nullptr);

		// Colored draws
		if (pipe2D && !coloredRanges.empty()) {
			SDL_BindGPUGraphicsPipeline(rp, pipe2D);
			SDL_GPUBufferBinding vb = { vbuf2D, 0 };
			SDL_BindGPUVertexBuffers(rp, 0, &vb, 1);

			for (const auto& r : coloredRanges) {
				float col[4] = { r.r, r.g, r.b, r.a };
				SDL_PushGPUFragmentUniformData(cmd, 0, col, sizeof(col));
				SDL_DrawGPUPrimitives(rp, r.count, 1, r.first, 0);
			}
		}

		// Textured draws
		if (pipeTex && !texRanges.empty()) {
			SDL_BindGPUGraphicsPipeline(rp, pipeTex);
			SDL_GPUBufferBinding vb = { vbufTex, 0 };
			SDL_BindGPUVertexBuffers(rp, 0, &vb, 1);

			for (const auto& r : texRanges) {
				SDL_GPUTextureSamplerBinding tsb = { r.tex->GetGPUTexture(), r.tex->GetSampler() };
				SDL_BindGPUFragmentSamplers(rp, 0, &tsb, 1);
				float col[4] = { r.r, r.g, r.b, r.a };
				SDL_PushGPUFragmentUniformData(cmd, 0, col, sizeof(col));
				SDL_DrawGPUPrimitives(rp, r.count, 1, r.first, 0);
			}
		}

		SDL_EndGPURenderPass(rp);
		SDL_SubmitGPUCommandBuffer(cmd);
		queue.clear();
	}

}
