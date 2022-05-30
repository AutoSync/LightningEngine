#pragma once
#ifndef EQNX_SHADER
#define EQNX_SHADER

#include "equinox-handler.h"
#include "random-rankit.h"
#include <GLEW/glew.h>
#include <glm/glm.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

using namespace Equinox;
namespace Equinox
{
	class Shader
	{
	public:
		Shader(Equinox::Handler* hdl);
		Shader(const char* vertPath, const char* fragPath);
		Shader();
		~Shader();
	public:
		unsigned int getID();
		void CreateShader(Equinox::Handler* hdl);
		void CreateShader(const char* vertpath, const char* fragpath);
		void use() const;
		//Set Uniforms bool
		void setBool(const std::string& name, bool value) const;
		//Set Uniforms int
		void setInt(const std::string& name, int value) const;
		//Set Uniforms float
		void setFloat(const std::string& name, float value) const;
		//Set Uniforms in float X, float y
		void setVec2(const std::string& name, float x, float y) const;
		//Set Uniforms in Vec2
		void setVec2(const std::string& name, const glm::vec2& value) const;
		//Set Uniforms in float X, float y, float z
		void setVec3(const std::string& name, float x, float y, float z) const;
		//Set Uniforms in Vec3
		void setVec3(const std::string& name, const glm::vec3& value) const;
		//Set Uniforms in all vectors 3
		void setVec3(const std::string& name, float scalar) const;
		//Set Uniforms with RGB for Vec3
		void setVec3(const std::string& name, const Random::Structs::c3 value) const;
		//Set Uniforms with V3 for Vectors 3
		void setVec3(const std::string& name, const Random::Structs::v3 value) const;
		//Set Uniforms in Float x, float y, float z, float w
		void setVec4(const std::string& name, float x, float y, float z, float w) const;
		//Set Uniforms in Vec4
		void setVec4(const std::string& name, const glm::vec4& value) const;
		//Set Uniforms with RGBA for Vec4
		void setVec4(const std::string& name, const Random::Structs::c4 value) const;
		//Set Uniforms with V4 for Vec4
		void setVec4(const std::string& name, const Random::Structs::v4 value) const;
		//Set Uniforms in all vectors 4
		void setVec4(const std::string& name, float scalar);
		//Set Uniforms in Matrix 2x2
		void setMat2(const std::string& name, const glm::mat2& mat2) const;
		//Set Uniforms in Matrix 3x3
		void setMat3(const std::string& name, const glm::mat3& mat3) const;
		//Set Uniforms in Matrix 4x4
		void setMat4(const std::string& name, const glm::mat4& mat4) const;
		std::string matrixString(const std::string arrayName, int Position, const std::string member);
		
	private:
		unsigned int ID = 0;
		void buildShader(const char* vertpath, const char* fragpath);
		void eqnx_msgError(GLint shader, std::string type);
	};
}

#endif // !EQNX_SHADER

