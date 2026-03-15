// Renderer — SDL_GPU-backed 2D renderer.
// Does NOT own the SDL_GPUDevice or SDL_Window — lifetime managed by Window.
// Uses a command queue: draw calls are recorded in Clear()..Present() and
// flushed as a single GPU render pass in Present().
//
// CLASS HIERARCHY:
// WINDOW -> GAMEINSTANCE -> LEVEL -> NODES -> [ RENDERER (utility) ]

#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <vector>
#include "Texture.h"

namespace LightningEngine {

	class Renderer {
	private:
		SDL_GPUDevice*           device     = nullptr;
		SDL_Window*              window     = nullptr;

		SDL_FColor               clearColor = { 0.f, 0.f, 0.f, 1.f };

		// Colored 2D pipeline (alpha-blended)
		SDL_GPUGraphicsPipeline* pipe2D     = nullptr;
		SDL_GPUBuffer*           vbuf2D     = nullptr;

		// Textured 2D pipeline (alpha-blended)
		SDL_GPUGraphicsPipeline* pipeTex    = nullptr;
		SDL_GPUBuffer*           vbufTex    = nullptr;

		static constexpr Uint32 kMaxVerts    = 4096;
		static constexpr Uint32 kMaxTexVerts = 4096;

		// Per-frame draw state
		float dr = 1.f, dg = 1.f, db = 1.f, da = 1.f;

		struct DrawCmd {
			float    x, y, w, h;
			float    r, g, b, a;
			bool     fill;
			Texture* texture; // nullptr = colored
			float    u0, v0, u1, v1;
		};
		std::vector<DrawCmd> queue;

		bool           initPipeline2D();
		bool           initPipelineTex();
		SDL_GPUShader* loadShader(SDL_GPUShaderStage stage, const char* path,
		                          Uint32 numSamplers, Uint32 numUniformBuffers);
		void           addQuad(std::vector<float>& v, float x1, float y1, float x2, float y2);
		void           addTexQuad(std::vector<float>& v,
		                          float x1, float y1, float x2, float y2,
		                          float u0, float v0, float u1, float v1);

	public:
		Renderer() = default;
		Renderer(SDL_GPUDevice* dev, SDL_Window* win);

		// Release GPU-owned resources — must be called before SDL_DestroyGPUDevice.
		void Release();

		// --- Load a texture from file (BMP natively; PNG/JPG requires stb_image) ---
		Texture LoadTexture(const char* path);

		// --- Frame API ---
		void SetClearColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);
		void Clear();
		void Present();

		// --- Draw API ---
		void SetDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);
		void DrawRect(float x, float y, float w, float h);
		void FillRect(float x, float y, float w, float h);

		// Draw texture at (x,y) with size (w,h). w=0/h=0 → natural size.
		void DrawTexture(Texture& tex, float x, float y, float w = 0.f, float h = 0.f);
		// Draw a sub-region of a texture (for sprite sheets).
		void DrawTextureRegion(Texture& tex, float x, float y, float w, float h,
		                       float u0, float v0, float u1, float v1);

		// Internal — for engine backends only.
		SDL_GPUDevice* GetDevice() const { return device; }
	};

}
