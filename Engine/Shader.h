#pragma once
//Engine
#include "Engine.h"
#include "Import.h"
#include "System.h"

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
	enum TypeProgram
	{
		VERTEX, FRAGMENT, PROGRAM, GEOMETRY
	};
	class Shader
	{
	private:
		ShaderSource source;
	public:
		uint id = 0;
	public:
		Shader();
		Shader(ShaderSource _source);
		Shader(ShaderType type, ShaderSource _source);
		Shader(const char* vert_path, const char* frag_path, const char* geo_path = " ");
		Shader(const char* vert_path, const char* frag_path, bool debug ,const char* geo_path = " ");
		void Render();
		void SetBool(const char* name, bool value);
		void SetInt(const char* name, int value);
		void SetFloat(const char* name, float value);
		void SetV2(const char* name, V2 value);
		void SetV3(const char* name, V3 value);
		void SetLinearColor(const char* name, LinearColor value);
		void SetColor(const char* name, C3 value);
		void SetV4(const char* name, V4 value);
		void SetMat2(const char* name, glm::mat2& m);
		void SetMat3(const char* name, glm::mat3& m);
		void SetMat4(const char* name, glm::mat4& m);
		const char* MatrixChars(const char* ArrayName, int Position, const char* member);
		void Projection();
		void View();
		void Model();

	private:
		void InitializeShader(ShaderSource _source);
		void ShaderMessageError(GLint shader, TypeProgram type);
	};
	void LoadShader(Shader& shader, const char* vertex_path, const char* fragment_path, const char* geometry_path = "");
	void LoadShader(ShaderSource& source, const char* vertex_path, const char* fragment_path, const char* geometry_path = "");
}

