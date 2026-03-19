#include "../include/Framebuffer.h"
#include "../include/Renderer.h"
#include <iostream>

namespace LightningEngine {

bool Framebuffer::Create(Renderer& r, int w, int h)
{
	device = r.GetDevice();
	if (!device || w <= 0 || h <= 0) return false;

	fbW = w;
	fbH = h;

	// --- GPU texture: COLOR_TARGET (render to) + SAMPLER (read from) ---
	SDL_GPUTextureCreateInfo texInfo = {};
	texInfo.type                 = SDL_GPU_TEXTURETYPE_2D;
	texInfo.format               = kFormat;
	texInfo.usage                = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET
	                             | SDL_GPU_TEXTUREUSAGE_SAMPLER;
	texInfo.width                = (Uint32)w;
	texInfo.height               = (Uint32)h;
	texInfo.layer_count_or_depth = 1;
	texInfo.num_levels           = 1;

	fbTex = SDL_CreateGPUTexture(device, &texInfo);
	if (!fbTex) {
		std::cerr << "[Framebuffer] SDL_CreateGPUTexture failed: " << SDL_GetError() << "\n";
		return false;
	}

	// --- Sampler (linear, clamp) ---
	SDL_GPUSamplerCreateInfo sampInfo = {};
	sampInfo.min_filter     = SDL_GPU_FILTER_LINEAR;
	sampInfo.mag_filter     = SDL_GPU_FILTER_LINEAR;
	sampInfo.mipmap_mode    = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
	sampInfo.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	sampInfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	sampInfo.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;

	sampler = SDL_CreateGPUSampler(device, &sampInfo);
	if (!sampler) {
		std::cerr << "[Framebuffer] SDL_CreateGPUSampler failed: " << SDL_GetError() << "\n";
		SDL_ReleaseGPUTexture(device, fbTex);
		fbTex = nullptr;
		return false;
	}

	// Non-owning Texture view (device=nullptr → Release() is a no-op)
	view = Texture::ViewOf(fbTex, sampler, w, h);
	return true;
}

bool Framebuffer::Resize(Renderer& r, int newW, int newH)
{
	if (newW == fbW && newH == fbH && IsValid()) return true;
	Release();
	return Create(r, newW, newH);
}

void Framebuffer::Release()
{
	view = Texture{}; // reset view (no GPU release — device is nullptr in view)
	if (device) {
		if (sampler) { SDL_ReleaseGPUSampler(device, sampler); sampler = nullptr; }
		if (fbTex)   { SDL_ReleaseGPUTexture(device, fbTex);   fbTex   = nullptr; }
	}
	device = nullptr;
	fbW    = 0;
	fbH    = 0;
}

} // namespace LightningEngine
