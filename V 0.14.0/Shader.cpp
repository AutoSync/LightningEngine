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

Lightning::Shader::Shader(const char* vert_path, const char* frag_path, const char* geo_path)
{
	/*const char* vertex = ReadFile(vert_path, "VERTEX SHADER");
	const char* fragment = ReadFile(frag_path, "FRAGMENT SHADER");*/
	std::string vertexSource, fragmentSource;
	std::fstream vertexFile, fragmentFile;

	vertexFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fragmentFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		vertexFile.open(vert_path);
		fragmentFile.open(frag_path);

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

	uint vertexShader, fragmentShader;
	int success = 0;
	char infoLog[512] = { ' ' };
	//Vertex Shader
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &Vshader, NULL);
	glCompileShader(vertexShader);
	ShaderMessageError(vertexShader, "VERTEX");
	//Fragment Shader
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &Fshader, NULL);
	glCompileShader(fragmentShader);
	ShaderMessageError(fragmentShader, "FRAGMENT");
	//Shader Program
	id = glCreateProgram();
	glAttachShader(id, vertexShader);
	glAttachShader(id, fragmentShader);
	glLinkProgram(id);
	ShaderMessageError(id, "PROGRAM");

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);	
}

void Lightning::Shader::Render()
{
	glUseProgram(id);
}

void Lightning::Shader::SetBool(const char* name, bool value)
{
	glUniform1i(glGetUniformLocation(id, name), (int)value);
}

void Lightning::Shader::SetInt(const char* name, int value)
{
	glUniform1i(glGetUniformLocation(id, name), value);
}

void Lightning::Shader::SetFloat(const char* name, float value)
{
	glUniform1f(glGetUniformLocation(id, name), value);
}

void Lightning::Shader::SetV2(const char* name, V2 value)
{
	glUniform2f(glGetUniformLocation(id, name), value.x, value.y);
}

void Lightning::Shader::SetV3(const char* name, V3 value)
{
	glUniform3f(glGetUniformLocation(id, name), value.x, value.y, value.z);
}

void Lightning::Shader::SetLinearColor(const char* name, LinearColor value)
{
	glUniform3f(glGetUniformLocation(id, name), value.r, value.g, value.b);
}

void Lightning::Shader::SetColor(const char* name, C3 value)
{
	glUniform3f(glGetUniformLocation(id, name), value.r, value.g, value.b);
}

void Lightning::Shader::SetV4(const char* name, V4 value)
{
	glUniform4f(glGetUniformLocation(id, name), value.x, value.y, value.z, value.w);
}

void Lightning::Shader::SetMat2(const char* name, glm::mat2& m)
{
	glUniformMatrix2fv(glGetUniformLocation(id, name),1, GL_FALSE, &m[0][0]);
}

void Lightning::Shader::SetMat3(const char* name, glm::mat3& m)
{
	glUniformMatrix3fv(glGetUniformLocation(id, name), 1, GL_FALSE, &m[0][0]);
}

void Lightning::Shader::SetMat4(const char* name, glm::mat4& m)
{
	glUniformMatrix4fv(glGetUniformLocation(id, name), 1, GL_FALSE, &m[0][0]);
}

const char* Lightning::Shader::MatrixChars(const char* ArrayName, int Position, const char* member)
{
	string an = ArrayName;
	string mb = member;
	string data = an + "[" + std::to_string(Position) + "]" + "." + mb;
	return data.c_str();
}


void Lightning::Shader::InitializeShader(ShaderSource _source)
{
	source = ShaderSource(_source);
	uint vertex, fragment;
	int success = 0;
	char infoLog[512] = { ' ' };
	//Vertex Shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &source.vertex, NULL);
	glCompileShader(vertex);
	ShaderMessageError(vertex, "VERTEX");
	//Fragment Shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &source.fragment, NULL);
	glCompileShader(fragment);
	ShaderMessageError(fragment, "FRAGMENT");
	//Shader Program
	id = glCreateProgram();
	glAttachShader(id, vertex);
	glAttachShader(id, fragment);
	glLinkProgram(id);
	ShaderMessageError(id, "PROGRAM");
	// exclua os shaders como eles estão vinculados ao nosso programa agora e não são mais necessários
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

void Lightning::Shader::ShaderMessageError(GLint shader, string type)
{
	GLint success = 0;
	GLchar infoLog[1024] = { '0' };
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

void Lightning::LoadShader(Shader& shader, const char* vertex_path, const char* fragment_path, const char* geometry_path)
{
	const char* vertex = ReadFile(vertex_path, "VERTEX SHADER");
	const char* fragment = ReadFile(fragment_path, "FRAGMENT SHADER");

	

}

void Lightning::LoadShader(ShaderSource& source, const char* vertex_path, const char* fragment_path, const char* geometry_path)
{
	
}
