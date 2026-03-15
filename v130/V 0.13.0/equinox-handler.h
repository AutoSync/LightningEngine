#pragma once
#include <string>
namespace Equinox
{
	enum class RHI
	{
		OpenGL,
		DirectX,
		Vulkan
	};
	struct Handler
	{
		void free();
		void setRHI(RHI new_RHI);
		//says what kind of API the object holds
		RHI RenderHardwareInterface = RHI::OpenGL;
		std::string VertexShader;
		std::string GeometryShader;
		std::string FragmentShader;
	};
}
