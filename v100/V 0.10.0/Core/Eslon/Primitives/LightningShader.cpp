#include "LightningShader.h"

LightningShader::LightningShader(const char* vertexPath, const char* fragmentPath)
{
	// 1. recupera o código fonte do vértice / fragmento de filePath
	std::string vertexCode;
	std::string fragmentCode;
	std::fstream vShaderFile;
	std::fstream fShaderFile;
	// assegure que os objetos stream possam gerar exceções:
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		// Abrir arquivos
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		std::stringstream vShaderStream, fShaderStream;
		// lê o conteúdo do buffer do arquivo em fluxos
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		// fecha manipuladores de arquivos
		vShaderFile.close();
		fShaderFile.close();
		// converte o fluxo em string
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}
	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();
	// 2. compilar sombreadores
	unsigned int vertex, fragment;
	int success;
	char infoLog[512];
	//Vertex Shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	checkCompileErros(vertex, "VERTEX");
	//Fragment Shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	checkCompileErros(fragment, "FRAGMENT");
	//Shader Program
	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);
	checkCompileErros(ID, "PROGRAM");
	// exclua os shaders como eles estão vinculados ao nosso programa agora e não são mais necessários
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}
void LightningShader::use()
{
	glUseProgram(ID);
}
void LightningShader::setBool(const std::string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}
void LightningShader::setInt(const std::string& name, int value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
void LightningShader::setFloat(const std::string& name, float value) const
{
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}
void LightningShader::setVec2(const std::string& name, float x, float y) const
{
	glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
}
void LightningShader::setVec2(const std::string& name, const glm::vec2& value) const
{
	glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void LightningShader::setVec3(const std::string& name, float x, float y, float z) const
{
	glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}
void LightningShader::setVec3(const std::string& name, const glm::vec3& value) const
{
	glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void LightningShader::setVec4(const std::string& name, float x, float y, float z, float w) const
{
	glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
}
void LightningShader::setVec4(const std::string& name, const glm::vec4& value) const
{
	glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void LightningShader::setMat2(const std::string& name, const glm::mat2& mat2) const
{
	glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat2[0][0]);
}
void LightningShader::setMat3(const std::string& name, const glm::mat3& mat3) const
{
	glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat3[0][0]);
}
void LightningShader::setMat4(const std::string& name, const glm::mat4& mat4) const
{
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat4[0][0]);
}
void LightningShader::checkCompileErros(GLint shader, std::string type)
{
	GLint success;
	GLchar infoLog[1024];
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
std::string LightningShader::toArray(std::string ftext, std::string stext, int number)
{
	std::string firstArg, secondArg, ca, cf, ni, vt, NewArray;
	firstArg = ftext;
	secondArg = stext;
	ca = "[";
	cf = "]";
	ni = std::to_string(number);
	vt = ca + ni + cf;
	NewArray = firstArg + vt + secondArg;
	return NewArray;
}
