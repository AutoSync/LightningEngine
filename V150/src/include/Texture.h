// Texture — GPU-side 2D texture.
// Created via Renderer::LoadTexture(path). Released via Texture::Release().
// Supports BMP natively. PNG/JPG requires stb_image (add third_party/stb/stb_image.h).

#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

namespace LightningEngine {

	class Texture {
	private:
		SDL_GPUDevice*  device  = nullptr;
		SDL_GPUTexture* gpuTex  = nullptr;
		SDL_GPUSampler* sampler = nullptr;
		int             width   = 0;
		int             height  = 0;

	public:
		Texture() = default;

		// Upload RGBA8 pixel data directly — used internally by LoadTexture.
		static Texture FromPixels(SDL_GPUDevice* device, const void* rgba8, int w, int h, bool nearestFilter = false);

		void Release();

		bool IsValid()   const { return gpuTex != nullptr; }
		int  GetWidth()  const { return width;  }
		int  GetHeight() const { return height; }

		// Internal — for Renderer use only.
		SDL_GPUTexture* GetGPUTexture() const { return gpuTex;  }
		SDL_GPUSampler* GetSampler()    const { return sampler; }

		// Creates a non-owning view of an existing GPU texture (used by Framebuffer).
		// Release() on this view is a no-op — the owner manages lifetime.
		static Texture ViewOf(SDL_GPUTexture* tex, SDL_GPUSampler* samp, int w, int h)
		{
			Texture t;
			// device=nullptr → Release() skips GPU calls (no double-free)
			t.gpuTex  = tex;
			t.sampler = samp;
			t.width   = w;
			t.height  = h;
			return t;
		}
	};

}
