#ifndef LIGHTNINGSHADER_H
#define LIGHTNINGSHADER_H
//OpenGL
#include <glad/glad.h>
#include <glm/glm.hpp>
//standard libary
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>



class LightningShader
{
public:
	// the program ID
	unsigned int ID = 0;
	// constructor reads and builds the shader
	LightningShader(const char* vertexPath, const char* fragmentPath);
	// use/activate the shader
	void use();
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
	//Set Uniforms in Float x, float y, float z, float w
	void setVec4(const std::string& name, float x, float y, float z, float w) const;
	//Set Uniforms in Vec4
	void setVec4(const std::string& name, const glm::vec4& value) const;
	//Set Uniforms in Matrix 2x2
	void setMat2(const std::string& name, const glm::mat2& mat2) const;
	//Set Uniforms in Matrix 3x3
	void setMat3(const std::string& name, const glm::mat3& mat3) const;
	//Set Uniforms in Matrix 4x4
	void setMat4(const std::string& name, const glm::mat4& mat4) const;
private:
	void checkCompileErros(GLint shader, std::string type);
	std::string toArray(std::string ftext, std::string stext, int number);
};
#endif
