// Framebuffer.h — Off-screen GPU render target.
// Create once, then use Renderer::BeginRenderToTexture / EndRenderToTexture
// to redirect draw calls to this framebuffer. GetTexture() returns a
// Texture& suitable for DrawTexture / DrawTextureRegion.
//
// Usage:
//   Framebuffer fb;
//   fb.Create(renderer, 1280, 720);
//
//   // Each frame:
//   renderer.BeginRenderToTexture(fb);
//   renderer.FillRect(...);       // draws go to fb
//   renderer.DrawTexture(...);
//   renderer.EndRenderToTexture();
//
//   renderer.DrawTexture(fb.GetTexture(), 0, 0, 1280, 720); // blit to screen
//
//   fb.Release(); // on shutdown
#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include "Texture.h"

namespace LightningEngine {

class Renderer; // forward

class Framebuffer {
private:
	SDL_GPUDevice*       device  = nullptr;
	SDL_GPUTexture*      fbTex   = nullptr;
	SDL_GPUSampler*      sampler = nullptr;
	Texture              view;      // non-owning Texture view (for DrawTexture)
	int                  fbW = 0, fbH = 0;

	// Framebuffers always use RGBA8 — independent of swapchain format.
	static constexpr SDL_GPUTextureFormat kFormat = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;

public:
	Framebuffer() = default;

	// Allocate GPU resources. Call once (or after Release()).
	bool Create(Renderer& r, int w, int h);

	// Resize: releases old and creates new. Safe to call every frame if size changes.
	bool Resize(Renderer& r, int newW, int newH);

	// Free GPU resources.
	void Release();

	bool IsValid()   const { return fbTex != nullptr; }
	int  GetWidth()  const { return fbW; }
	int  GetHeight() const { return fbH; }

	// Returns a Texture view for use with DrawTexture / DrawTextureRegion.
	// Valid until the next Resize() or Release().
	Texture& GetTexture() { return view; }

	// Internal — used by Renderer only.
	SDL_GPUTexture*      GetGPUTexture() const { return fbTex;   }
	SDL_GPUSampler*      GetSamplerPtr() const { return sampler; }
	SDL_GPUTextureFormat GetFormat()     const { return kFormat;  }
};

} // namespace LightningEngine
