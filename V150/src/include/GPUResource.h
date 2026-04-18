#pragma once
#include <SDL3/SDL_gpu.h>

namespace LightningEngine {

template<typename T, void(*Deleter)(SDL_GPUDevice*, T*)>
class GPUHandle {
public:
	GPUHandle() = default;
	GPUHandle(SDL_GPUDevice* device, T* ptr, bool owning = true)
		: device_(device), ptr_(ptr), owning_(owning) {}

	GPUHandle(const GPUHandle&) = delete;
	GPUHandle& operator=(const GPUHandle&) = delete;

	GPUHandle(GPUHandle&& other) noexcept
		: device_(other.device_), ptr_(other.ptr_), owning_(other.owning_)
	{
		other.device_ = nullptr;
		other.ptr_ = nullptr;
		other.owning_ = true;
	}

	GPUHandle& operator=(GPUHandle&& other) noexcept
	{
		if (this != &other) {
			reset();
			device_ = other.device_;
			ptr_ = other.ptr_;
			owning_ = other.owning_;
			other.device_ = nullptr;
			other.ptr_ = nullptr;
			other.owning_ = true;
		}
		return *this;
	}

	~GPUHandle() { reset(); }

	void reset()
	{
		if (owning_ && device_ && ptr_)
			Deleter(device_, ptr_);
		device_ = nullptr;
		ptr_ = nullptr;
		owning_ = true;
	}

	void reset(SDL_GPUDevice* device, T* ptr, bool owning = true)
	{
		reset();
		device_ = device;
		ptr_ = ptr;
		owning_ = owning;
	}

	T* get() const { return ptr_; }
	SDL_GPUDevice* device() const { return device_; }
	bool owning() const { return owning_; }
	void setOwning(bool owning) { owning_ = owning; }

	explicit operator bool() const { return ptr_ != nullptr; }
	operator T*() const { return ptr_; }

private:
	SDL_GPUDevice* device_ = nullptr;
	T* ptr_ = nullptr;
	bool owning_ = true;
};

using UniquePipeline = GPUHandle<SDL_GPUGraphicsPipeline, SDL_ReleaseGPUGraphicsPipeline>;
using UniqueBuffer = GPUHandle<SDL_GPUBuffer, SDL_ReleaseGPUBuffer>;
using UniqueTexture = GPUHandle<SDL_GPUTexture, SDL_ReleaseGPUTexture>;
using UniqueSampler = GPUHandle<SDL_GPUSampler, SDL_ReleaseGPUSampler>;

} // namespace LightningEngine