#include "Shader.h"

Lightning::Shader::Shader()
{
	ShaderSource _source = ShaderSource();
	InitializeShader(_source);
}

Lightning::Shader::Shader(ShaderSource _source)
{
	InitializeShader(_source);
}

Lightning::Shader::Shader(ShaderType type, ShaderSource _source)
{
	switch (type)
	{
	case Lightning::Standard:
		InitializeShader(_source);
		break;
	case Lightning::Opaque:
		break;
	case Lightning::Translucent:
		break;
	case Lightning::PBR:
		break;
	case Lightning::Unlit:
		break;
	case Lightning::PostProcess:
		break;
	case Lightning::LightMaterial:
		break;
	}
}

void Lightning::Shader::Render()
{
	glUseProgram(Id);
}

void Lightning::Shader::SetBool(const char* name, bool value)
{
	glUniform1i(glGetUniformLocation(Id, name), (int)value);
}

void Lightning::Shader::SetInt(const char* name, int value)
{
	glUniform1i(glGetUniformLocation(Id, name), value);
}

void Lightning::Shader::SetFloat(const char* name, float value)
{
	glUniform1f(glGetUniformLocation(Id, name), value);
}

void Lightning::Shader::SetV2(const char* name, V2 value)
{
	glUniform2f(glGetUniformLocation(Id, name), value.x, value.y);
}

void Lightning::Shader::SetV3(const char* name, V3 value)
{
	glUniform3f(glGetUniformLocation(Id, name), value.x, value.y, value.z);
}

void Lightning::Shader::SetV4(const char* name, V4 value)
{
	glUniform4f(glGetUniformLocation(Id, name), value.x, value.y, value.z, value.w);
}

void Lightning::Shader::SetMat2(const char* name, glm::mat2& m)
{
	glUniformMatrix2fv(glGetUniformLocation(Id, name),1, GL_FALSE, &m[0][0]);
}

void Lightning::Shader::SetMat3(const char* name, glm::mat3& m)
{
	glUniformMatrix3fv(glGetUniformLocation(Id, name), 1, GL_FALSE, &m[0][0]);
}

void Lightning::Shader::SetMat4(const char* name, glm::mat4& m)
{
	glUniformMatrix4fv(glGetUniformLocation(Id, name), 1, GL_FALSE, &m[0][0]);
}


void Lightning::Shader::InitializeShader(ShaderSource _source)
{
	source = _source;
	uint vertex, fragment;
	int success;
	char infoLog[512];

	// vertex Shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &source.vertex, NULL);
	glCompileShader(vertex);
	// print compile errors if any
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		Msg::Emit(Flow::WARNING, "Error Vertex Shader Failed\n" + (string)infoLog);
	};

	//Fragment Shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &source.fragment, NULL);
	glCompileShader(fragment);
	// print compile errors if any
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		Msg::Emit(Flow::WARNING, "Error Fragment Shader Failed\n" + (string)infoLog);
	};

	// shader Program
	Id = glCreateProgram();
	glAttachShader(Id, vertex);
	glAttachShader(Id, fragment);
	glLinkProgram(Id);
	// print linking errors if any
	glGetProgramiv(Id, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(Id, 512, NULL, infoLog);
		Msg::Emit(Flow::WARNING, "Error Shader Program Linking Failed\n" + (string)infoLog);
	}

	// delete the shaders as they're linked into our program now and no longer necessary
	glDeleteShader(vertex);
	glDeleteShader(fragment);

}

Lightning::ShaderSource Lightning::LoadShader(const char* vertex_path, const char* fragment_path, const char* geometry_path)
{
	std::string vertexCode;
	std::string fragmentCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	// ensure ifstream objects can throw exceptions:
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		// open files
		vShaderFile.open(vertex_path);
		fShaderFile.open(fragment_path);
		std::stringstream vShaderStream, fShaderStream;
		// read file's buffer contents into streams
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		// close file handlers
		vShaderFile.close();
		fShaderFile.close();
		// convert stream into string
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
	}
	catch (std::ifstream::failure e)
	{
		Msg::Emit(Flow::WARNING, "Error Shader File not succesfully read");
		string message, vert, frag, geo;
		message = "Error to read Shaders: \n";
		vert = "VERT: " + (string)vertex_path + " \n";
		frag = "FRAG: " + (string)fragment_path + "\n";
		geo = "GEO: " + (string)geometry_path + "\n";

		message += vert + frag + geo;

		Msg::Emit(Flow::WARNING, message);
			
	} 
	ShaderSource temp;
	temp.vertex = vertexCode.c_str();
	temp.fragment = fragmentCode.c_str();

	return temp;
}
