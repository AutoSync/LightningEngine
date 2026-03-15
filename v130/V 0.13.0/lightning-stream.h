#pragma once
#include "random-rankit.h"
//#include "random-windows.h"
#include "equinox-handler.h"
#include "titan-handler.h"
#include "lightning-model.h"

//3rd Paty library
#include "Resources/3rdParty/stb_image.h"
#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
//#define GLFW_EXPOSE_NATIVE_WIN32
//#include <GLFW/glfw3native.h>

//standard library
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace Random::Enums;
using namespace Random::Structs;

namespace Lightning
{
	namespace Stream
	{
		class Importer
		{
		public:
			Importer();
			~Importer();
			//Imports and defines an Equinox Shader
			void Import(flag flags, const char* vertex_path, const char* fragment_path, Equinox::Handler* Handler);
			//Load textures into the 3d model
			uint TextureFromFile(solidtext path, const text& directory, bool gamma = false);
		};

		class Exporter
		{
			
		};
	}
}
