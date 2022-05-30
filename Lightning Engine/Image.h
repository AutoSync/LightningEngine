#pragma once
#include "Types.h"
#include "Shader.h"


namespace Lightning
{
	class Image
	{
	public:
		Image();
		void Import(const char* vertex_path, const char* fragment_path, ShaderSource* Handler);
		//Load textures into the 3d model
		uint TextureFromFile(const char* path, const string& directory, bool gamma = false);
		uint TextureFromFile(const char* path);
	};

}
