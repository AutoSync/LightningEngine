#pragma once
//Engine
#include "Engine.h"
//Standard Library
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
using namespace std;

namespace Lightning
{
	struct ShaderSource
	{
		const char* vertex;
		const char* fragment;
		const char* geometry;
		ShaderSource()
		{
			this->vertex = "";
			this->fragment = "";
			this->geometry = "";
		}
		ShaderSource(const char* vertex, const char* fragment, const char* geometry = "")
		{
			this->vertex = vertex;
			this->fragment = fragment;
			this->geometry = geometry;
		}
		void operator=(const ShaderSource& s)
		{
			this->vertex = s.vertex;
			this->fragment = s.fragment;
			this->geometry = s.geometry;
		}
	};
	enum ShaderType
	{
		Standard, Opaque, Translucent, PBR, Unlit, PostProcess, LightMaterial
	};
	class Shader
	{
	private:
		ShaderSource source;
	public:
		uint Id = 0;
	public:
		Shader();
		Shader(ShaderSource _source);
		Shader(ShaderType type, ShaderSource _source);
		void Render();
		void SetBool(const char* name, bool value);
		void SetInt(const char* name, int value);
		void SetFloat(const char* name, float value);
		void SetV2(const char* name, V2 value);
		void SetV3(const char* name, V3 value);
		void SetLinearColor(const char* name, LinearColor value);
		void SetV4(const char* name, V4 value);
		void SetMat2(const char* name, glm::mat2& m);
		void SetMat3(const char* name, glm::mat3& m);
		void SetMat4(const char* name, glm::mat4& m);
		const char* MatrixChars(const char* ArrayName, int Position, const char* member);

	private:
		void InitializeShader(ShaderSource _source);
	};

	ShaderSource LoadShader(const char* vertex_path, const char* fragment_path, const char* geometry_path = "");

}

