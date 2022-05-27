#include "equinox-bake-opengl.h"

Equinox::Shader::Shader(){}
Equinox::Shader::~Shader(){}

Equinox::Shader::Shader(Equinox::Handler* hdl)
{
	CreateShader(hdl);
}

Equinox::Shader::Shader(const char* vertPath, const char* fragPath)
{
	std::string vertexSource, fragmentSource;
	std::fstream vertexFile, fragmentFile;

	vertexFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fragmentFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		vertexFile.open(vertPath);
		fragmentFile.open(fragPath);

		std::stringstream vertexStream, fragmentStream;

		vertexStream << vertexFile.rdbuf();
		fragmentStream << fragmentFile.rdbuf();

		vertexFile.close();
		fragmentFile.close();

		vertexSource = vertexStream.str();
		fragmentSource = fragmentStream.str();
	}
	catch (std::ifstream::failure fail)
	{
		std::cout << "ERROR::EQUINOX::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}

	const char* Vshader = vertexSource.c_str();
	const char* Fshader = fragmentSource.c_str();
	
	buildShader(Vshader, Fshader);
}

unsigned int Equinox::Shader::getID()
{
	return ID;
}

void Equinox::Shader::CreateShader(Equinox::Handler* hdl)
{
	const char* VertexSource = hdl->VertexShader.c_str();
	const char* FragmentSource = hdl->FragmentShader.c_str();

	buildShader(VertexSource, FragmentSource);
}
void Equinox::Shader::CreateShader(const char* vertpath, const char* fragpath)
{
	buildShader(vertpath, fragpath);
}

void Equinox::Shader::use() const
{
	glUseProgram(ID);
}

void Equinox::Shader::setBool(const std::string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Equinox::Shader::setInt(const std::string& name, int value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Equinox::Shader::setFloat(const std::string& name, float value) const
{
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Equinox::Shader::setVec2(const std::string& name, float x, float y) const
{
	glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
}

void Equinox::Shader::setVec2(const std::string& name, const glm::vec2& value) const
{
	glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Equinox::Shader::setVec3(const std::string& name, float x, float y, float z) const
{
	glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}

void Equinox::Shader::setVec3(const std::string& name, const glm::vec3& value) const
{
	glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Equinox::Shader::setVec3(const std::string& name, float scalar) const
{
	glUniform3f(glGetUniformLocation(ID, name.c_str()), scalar, scalar, scalar);
}

void Equinox::Shader::setVec3(const std::string& name, const Random::Structs::c3 value) const
{
	glUniform3f(glGetUniformLocation(ID, name.c_str()), value.r, value.g, value.b);
}

void Equinox::Shader::setVec3(const std::string& name, const Random::Structs::v3 value) const
{
	glUniform3f(glGetUniformLocation(ID, name.c_str()), value.x, value.y, value.z);
}

void Equinox::Shader::setVec4(const std::string& name, float x, float y, float z, float w) const
{
	glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
}

void Equinox::Shader::setVec4(const std::string& name, const glm::vec4& value) const
{
	glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Equinox::Shader::setVec4(const std::string& name, const Random::Structs::c4 value) const
{
	glUniform4f(glGetUniformLocation(ID, name.c_str()), value.r, value.g, value.b, value.a);
}

void Equinox::Shader::setVec4(const std::string& name, const Random::Structs::v4 value) const
{
	glUniform4f(glGetUniformLocation(ID, name.c_str()), value.x, value.y, value.z, value.w);
}

void Equinox::Shader::setVec4(const std::string& name, float scalar)
{
	glUniform4f(glGetUniformLocation(ID, name.c_str()), scalar, scalar, scalar, scalar);
}

void Equinox::Shader::setMat2(const std::string& name, const glm::mat2& mat2) const
{
	glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat2[0][0]);
}

void Equinox::Shader::setMat3(const std::string& name, const glm::mat3& mat3) const
{
	glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat3[0][0]);
}

void Equinox::Shader::setMat4(const std::string& name, const glm::mat4& mat4) const
{
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat4[0][0]);
}

std::string Equinox::Shader::matrixString(const std::string arrayName, int Position, const std::string member)
{
	return arrayName + "[" + std::to_string(Position) + "]" + "." + member;
}

void Equinox::Shader::buildShader(const char* vertpath, const char* fragpath)
{
	unsigned int vertex, fragment;
	int success = 0;
	char infoLog[512] = { ' ' };
	//Vertex Shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vertpath, NULL);
	glCompileShader(vertex);
	eqnx_msgError(vertex, "VERTEX");
	//Fragment Shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fragpath, NULL);
	glCompileShader(fragment);
	eqnx_msgError(fragment, "FRAGMENT");
	//Shader Program
	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);
	eqnx_msgError(ID, "PROGRAM");
	// exclua os shaders como eles estão vinculados ao nosso programa agora e não são mais necessários
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

void Equinox::Shader::eqnx_msgError(GLint shader, std::string type)
{
	GLint success = 0;
	GLchar infoLog[1024] = {'0'};
	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
}
