// Texture — GPU-side 2D texture.
// Created via Renderer::LoadTexture(path). Released via Texture::Release().
// Supports PNG/JPG/BMP via WIC on Windows; BMP remains available via SDL fallback.

#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include "GPUResource.h"

namespace LightningEngine {

	class Texture {
	private:
		SDL_GPUDevice*  device  = nullptr;
		UniqueTexture   gpuTex;
		UniqueSampler   sampler;
		bool            ownsGpu = true;
		int             width   = 0;
		int             height  = 0;

	public:
		Texture() = default;

		// Upload RGBA8 pixel data directly — used internally by LoadTexture.
		static Texture FromPixels(SDL_GPUDevice* device, const void* rgba8, int w, int h, bool nearestFilter = false);

		void Release();

		bool IsValid()   const { return static_cast<bool>(gpuTex); }
		int  GetWidth()  const { return width;  }
		int  GetHeight() const { return height; }

		// Internal — for Renderer use only.
		SDL_GPUTexture* GetGPUTexture() const { return gpuTex.get();  }
		SDL_GPUSampler* GetSampler()    const { return sampler.get(); }

		// Creates a non-owning view of an existing GPU texture (used by Framebuffer).
		// Release() on this view only clears local handles — the owner manages lifetime.
		static Texture ViewOf(SDL_GPUTexture* tex, SDL_GPUSampler* samp, int w, int h)
		{
			Texture t;
			t.ownsGpu = false;
			t.gpuTex.reset(nullptr, tex, false);
			t.sampler.reset(nullptr, samp, false);
			t.width   = w;
			t.height  = h;
			return t;
		}
	};

}
