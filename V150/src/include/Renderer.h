// Renderer — SDL_GPU-backed 2D/3D renderer.
// Does NOT own the SDL_GPUDevice or SDL_Window — lifetime managed by Window.
// Uses a command queue: draw calls are recorded in Clear()..Present() and
// flushed as a single GPU render pass in Present().
//
// 2D API:
//   SetDrawColor / FillRect / DrawRect / DrawLine / DrawCircle / FillCircle
//   DrawTexture / DrawTextureRegion / DrawTextureEx
//
// 3D API:
//   UploadMesh / ReleaseMesh / DrawMesh
//   SetView / SetProjection / SetLight
//   3D pass runs before the 2D pass; 2D draws are composited on top.
//
// Framebuffer support:
//   BeginRenderToTexture(fb) — redirects all draw calls to the framebuffer.
//   EndRenderToTexture()     — flushes to framebuffer, resumes swapchain mode.
//
// CLASS HIERARCHY:
// WINDOW -> GAMEINSTANCE -> LEVEL -> NODES -> [ RENDERER (utility) ]

#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <vector>
#include <glm/glm.hpp>
#include "Texture.h"
#include "Framebuffer.h"
#include "Mesh.h"

namespace LightningEngine {

	// ── 3D scene light ────────────────────────────────────────────────────────
	struct Light3D {
		glm::vec4 matColor   = { 1.f, 1.f, 1.f, 1.f };  // base material tint
		glm::vec4 lightDir   = { 0.f,-1.f, 0.f, 0.f };  // world-space direction (unnormalised ok)
		glm::vec4 lightColor = { 1.f, 1.f, 1.f, 1.f };  // rgb=colour, a=intensity
		glm::vec4 ambient    = { 0.2f,0.2f,0.2f,1.f };  // additive ambient
	};

	class Renderer {
	private:
		SDL_GPUDevice*           device     = nullptr;
		SDL_Window*              window     = nullptr;

		SDL_FColor               clearColor = { 0.f, 0.f, 0.f, 1.f };

		// ── Swapchain pipelines (swapchain format, determined at runtime) ──
		SDL_GPUGraphicsPipeline* pipe2D     = nullptr;
		SDL_GPUGraphicsPipeline* pipeTex    = nullptr;
		SDL_GPUBuffer*           vbuf2D     = nullptr;
		SDL_GPUBuffer*           vbufTex    = nullptr;

		// ── Render-target pipelines (R8G8B8A8_UNORM, for Framebuffer) ──
		SDL_GPUGraphicsPipeline* pipe2D_rt  = nullptr;
		SDL_GPUGraphicsPipeline* pipeTex_rt = nullptr;
		SDL_GPUBuffer*           vbuf2D_rt  = nullptr;
		SDL_GPUBuffer*           vbufTex_rt = nullptr;

		// ── Blur pipeline (textured, R8G8B8A8, separable gaussian) ──
		SDL_GPUGraphicsPipeline* pipeBlur   = nullptr;

		// ── 3D pipeline ───────────────────────────────────────────────────────
		SDL_GPUGraphicsPipeline* pipe3D     = nullptr;
		SDL_GPUTexture*          depthTex   = nullptr;
		Uint32                   depthW     = 0;
		Uint32                   depthH     = 0;

		glm::mat4  viewMat    = glm::mat4(1.f);
		glm::mat4  projMat    = glm::mat4(1.f);
		Light3D    sceneLight;
		Texture    whiteTex;    // 1×1 white fallback for unlit meshes

		struct Mesh3DCmd { Mesh* mesh; glm::mat4 model; Texture* albedo; };
		std::vector<Mesh3DCmd> mesh3DQueue;

		SDL_GPUTextureFormat     swapFmt    = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;

		static constexpr Uint32 kMaxVerts    = 65536;
		static constexpr Uint32 kMaxTexVerts = 65536;

		// Per-frame draw state
		float dr = 1.f, dg = 1.f, db = 1.f, da = 1.f;
		bool  screenSpace = false;

		// Camera world offset
		float camX = 0.f, camY = 0.f;

		struct DrawCmd {
			float    x, y, w, h;
			float    r, g, b, a;
			bool     fill;
			Texture* texture;
			float    u0, v0, u1, v1;
			bool     ss;          // screen-space flag (ignores camera offset)
			float    angle = 0.f; // rotation in degrees around pivot
			float    pivX  = 0.f; // pivot screen-space X
			float    pivY  = 0.f; // pivot screen-space Y
			bool     isCircle = false; // filled circle (triangle fan)
			int      segs     = 0;     // circle segments (0 = auto 32)
		};

		// Active queue — points to mainQueue normally, fbQueue during FB render.
		std::vector<DrawCmd>  mainQueue;
		std::vector<DrawCmd>  fbQueue;
		std::vector<DrawCmd>* activeQueue = &mainQueue;

		// Framebuffer target state
		SDL_GPUTexture* fbTarget   = nullptr;
		int             fbTargetW  = 0;
		int             fbTargetH  = 0;

		// ── Pipeline init helpers ──
		bool           initPipeline2D();
		bool           initPipelineTex();
		bool           initPipelinesRT();
		bool           initPipelineBlur();
		bool           initPipeline3D();

		SDL_GPUShader* loadShader(SDL_GPUShaderStage stage, const char* path,
		                          Uint32 numSamplers, Uint32 numUniformBuffers);

		SDL_GPUGraphicsPipeline* makePipeline2D (SDL_GPUTextureFormat fmt,
		                                         SDL_GPUShader* vert, SDL_GPUShader* frag);
		SDL_GPUGraphicsPipeline* makePipelineTex(SDL_GPUTextureFormat fmt,
		                                         SDL_GPUShader* vert, SDL_GPUShader* frag);
		SDL_GPUGraphicsPipeline* makePipeline3D (SDL_GPUTextureFormat fmt,
		                                         SDL_GPUShader* vert, SDL_GPUShader* frag);

		void ensureDepthBuffer(Uint32 w, Uint32 h);

		void addQuad   (std::vector<float>& v, float x1, float y1, float x2, float y2);
		void addTexQuad(std::vector<float>& v,
		                float x1, float y1, float x2, float y2,
		                float u0, float v0, float u1, float v1);

		struct FlushParams {
			SDL_GPUTexture*          target;
			Uint32                   tw, th;
			SDL_GPUBuffer*           vb2D;
			SDL_GPUBuffer*           vbTex;
			SDL_GPUGraphicsPipeline* p2D;
			SDL_GPUGraphicsPipeline* pTex;
			bool                     clear;
			SDL_FColor               clearColor;
		};
		void flushQueueTo(SDL_GPUCommandBuffer* cmd,
		                  const std::vector<DrawCmd>& q,
		                  const FlushParams& p);

		// Render all queued 3D meshes to swapTex, clearing it first.
		void render3DPass(SDL_GPUCommandBuffer* cmd,
		                  SDL_GPUTexture* swapTex, Uint32 sw, Uint32 sh);

	public:
		Renderer() = default;
		Renderer(SDL_GPUDevice* dev, SDL_Window* win);

		void Release();

		// --- Texture loading ---
		Texture LoadTexture(const char* path);
		Texture LoadTextureFromPixels(const void* rgba8, int w, int h, bool nearestFilter = false);

		// --- Frame API ---
		void SetClearColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);
		void Clear();
		void Present();

		// --- Framebuffer API ---
		void BeginRenderToTexture(Framebuffer& fb);
		void EndRenderToTexture();
		void BlurTexture(Framebuffer& dest, Texture& src, float radius = 4.f);

		// --- Camera ---
		void  SetCameraOffset(float x, float y) { camX = x; camY = y; }
		float GetCameraX() const { return camX; }
		float GetCameraY() const { return camY; }

		void BeginScreenSpace() { screenSpace = true;  }
		void EndScreenSpace()   { screenSpace = false; }

		// --- 2D Draw API ---
		void SetDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);
		void DrawRect(float x, float y, float w, float h);
		void FillRect(float x, float y, float w, float h);
		void DrawTexture(Texture& tex, float x, float y, float w = 0.f, float h = 0.f);
		void DrawTextureRegion(Texture& tex, float x, float y, float w, float h,
		                       float u0, float v0, float u1, float v1);

		// Rotate tex around pivot (screen-space px,py), angle in degrees.
		void DrawTextureEx(Texture& tex, float x, float y, float w, float h,
		                   float angle, float pivX, float pivY);

		// Thin line from (x1,y1) to (x2,y2). thickness in pixels.
		void DrawLine(float x1, float y1, float x2, float y2, float thickness = 1.f);

		// Circle outline: segs segments (0 = auto).
		void DrawCircle(float cx, float cy, float radius, int segs = 0);

		// Filled circle (triangle fan).
		void FillCircle(float cx, float cy, float radius, int segs = 0);

		// --- 3D API ---
		// Upload CPU mesh to GPU. Must be called before DrawMesh.
		void UploadMesh(Mesh& mesh);
		// Release GPU buffers (CPU data retained).
		void ReleaseMesh(Mesh& mesh);
		// Queue mesh for the next 3D render pass.
		// albedo = nullptr → uses white fallback texture.
		void DrawMesh(Mesh& mesh, const glm::mat4& model, Texture* albedo = nullptr);

		// Set view / projection matrices for the 3D pass.
		void SetView(const glm::mat4& v)       { viewMat    = v; }
		void SetProjection(const glm::mat4& p) { projMat    = p; }
		void SetLight(const Light3D& l)        { sceneLight = l; }

		// Get current view / projection (needed for FrustumCuller, etc.)
		const glm::mat4& GetView()       const { return viewMat; }
		const glm::mat4& GetProjection() const { return projMat; }

		// Internal — for engine backends only.
		SDL_GPUDevice*       GetDevice()          const { return device; }
		SDL_Window*          GetWindow()           const { return window; }
		SDL_GPUTextureFormat GetSwapchainFormat()  const { return swapFmt; }
	};

}
