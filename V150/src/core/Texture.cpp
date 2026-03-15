#include "../include/Texture.h"
#include <iostream>

namespace LightningEngine {

	Texture Texture::FromPixels(SDL_GPUDevice* device, const void* rgba8, int w, int h)
	{
		Texture tex;
		tex.device = device;
		tex.width  = w;
		tex.height = h;

		// --- GPU texture ---
		SDL_GPUTextureCreateInfo texInfo = {};
		texInfo.type                 = SDL_GPU_TEXTURETYPE_2D;
		texInfo.format               = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
		texInfo.usage                = SDL_GPU_TEXTUREUSAGE_SAMPLER;
		texInfo.width                = (Uint32)w;
		texInfo.height               = (Uint32)h;
		texInfo.layer_count_or_depth = 1;
		texInfo.num_levels           = 1;

		tex.gpuTex = SDL_CreateGPUTexture(device, &texInfo);
		if (!tex.gpuTex) {
			std::cerr << "[Texture] SDL_CreateGPUTexture failed: " << SDL_GetError() << "\n";
			return tex;
		}

		// --- Sampler ---
		SDL_GPUSamplerCreateInfo sampInfo = {};
		sampInfo.min_filter     = SDL_GPU_FILTER_LINEAR;
		sampInfo.mag_filter     = SDL_GPU_FILTER_LINEAR;
		sampInfo.mipmap_mode    = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
		sampInfo.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
		sampInfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
		sampInfo.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;

		tex.sampler = SDL_CreateGPUSampler(device, &sampInfo);
		if (!tex.sampler) {
			std::cerr << "[Texture] SDL_CreateGPUSampler failed: " << SDL_GetError() << "\n";
			SDL_ReleaseGPUTexture(device, tex.gpuTex);
			tex.gpuTex = nullptr;
			return tex;
		}

		// --- Upload pixels via transfer buffer + copy pass ---
		Uint32 dataSize = (Uint32)(w * h * 4);

		SDL_GPUTransferBufferCreateInfo tbInfo = {};
		tbInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
		tbInfo.size  = dataSize;

		SDL_GPUTransferBuffer* tb = SDL_CreateGPUTransferBuffer(device, &tbInfo);
		if (!tb) {
			std::cerr << "[Texture] SDL_CreateGPUTransferBuffer failed: " << SDL_GetError() << "\n";
			tex.Release();
			return tex;
		}

		void* map = SDL_MapGPUTransferBuffer(device, tb, false);
		SDL_memcpy(map, rgba8, dataSize);
		SDL_UnmapGPUTransferBuffer(device, tb);

		SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);
		SDL_GPUCopyPass*      cp  = SDL_BeginGPUCopyPass(cmd);

		SDL_GPUTextureTransferInfo src = {};
		src.transfer_buffer = tb;
		src.pixels_per_row  = (Uint32)w;
		src.rows_per_layer  = (Uint32)h;

		SDL_GPUTextureRegion dst = {};
		dst.texture = tex.gpuTex;
		dst.w       = (Uint32)w;
		dst.h       = (Uint32)h;
		dst.d       = 1;

		SDL_UploadToGPUTexture(cp, &src, &dst, false);
		SDL_EndGPUCopyPass(cp);
		SDL_SubmitGPUCommandBuffer(cmd);

		SDL_WaitForGPUIdle(device); // ensure upload completes before releasing transfer buffer
		SDL_ReleaseGPUTransferBuffer(device, tb);

		return tex;
	}

	void Texture::Release()
	{
		if (device) {
			if (sampler) { SDL_ReleaseGPUSampler(device, sampler); sampler = nullptr; }
			if (gpuTex)  { SDL_ReleaseGPUTexture(device, gpuTex);  gpuTex  = nullptr; }
		}
		device = nullptr;
		width  = 0;
		height = 0;
	}

}
