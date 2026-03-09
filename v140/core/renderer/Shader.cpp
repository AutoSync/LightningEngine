#include "../../Shader.h"

LightningEngine::Shader::Shader()
{
	ShaderSource _source = ShaderSource();
	InitializeShader(_source);
}

LightningEngine::Shader::Shader(ShaderSource _source)
{
	InitializeShader(_source);
}

LightningEngine::Shader::Shader(ShaderType type, ShaderSource _source)
{
	switch (type)
	{
	case LightningEngine::Standard:
		InitializeShader(_source);
		break;
	case LightningEngine::Opaque:
		break;
	case LightningEngine::Translucent:
		break;
	case LightningEngine::PBR:
		break;
	case LightningEngine::Unlit:
		break;
	case LightningEngine::PostProcess:
		break;
	case LightningEngine::LightMaterial:
		break;
	}
}

LightningEngine::Shader::Shader(const char* vert_path, const char* frag_path, const char* geo_path)
{
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
		std::cout << "ERROR TO READ SHADER FILE: " << std::endl;
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
	ShaderMessageError(vertexShader, TypeProgram::VERTEX);
	//Fragment Shader
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &Fshader, NULL);
	glCompileShader(fragmentShader);
	ShaderMessageError(fragmentShader, TypeProgram::FRAGMENT);
	//Shader Program
	id = glCreateProgram();
	glAttachShader(id, vertexShader);
	glAttachShader(id, fragmentShader);
	glLinkProgram(id);
	ShaderMessageError(id, TypeProgram::PROGRAM);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);	
}

LightningEngine::Shader::Shader(const char* vert_path, const char* frag_path, bool debug, const char* geo_path)
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
		std::cout << "ERROR TO READ SHADER FILE: " << std::endl;
	}

	const char* Vshader = vertexSource.c_str();
	const char* Fshader = fragmentSource.c_str();
	if (debug)
	{
		int i = 0;
		std::cout << "DEBUG VERTEX SOURCE \n";
		while (Vshader[i] != '\0')
		{
			std:: cout << Vshader[i];
			i++;
		}

		std::cout << endl;
		i = 0;

		std::cout << "DEBUG FRAGMENT SOURCE \n";
		while (Fshader[i] != '\0')
		{
			std::cout << Fshader[i];
			i++;
		}
		std::cout << endl;
	}

	uint vertexShader, fragmentShader;
	int success = 0;
	char infoLog[512] = { ' ' };
	//Vertex Shader
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &Vshader, NULL);
	glCompileShader(vertexShader);
	ShaderMessageError(vertexShader, TypeProgram::VERTEX);
	//Fragment Shader
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &Fshader, NULL);
	glCompileShader(fragmentShader);
	ShaderMessageError(fragmentShader, TypeProgram::FRAGMENT);
	//Shader Program
	id = glCreateProgram();
	glAttachShader(id, vertexShader);
	glAttachShader(id, fragmentShader);
	glLinkProgram(id);
	ShaderMessageError(id, TypeProgram::PROGRAM);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

void LightningEngine::Shader::Init()
{
	glUseProgram(id);
	started = true;
}

void LightningEngine::Shader::SetBool(const char* name, bool value)
{
	ShaderStarted();
	glUniform1i(glGetUniformLocation(id, name), (int)value);
}

void LightningEngine::Shader::SetInt(const char* name, int value)
{
	ShaderStarted();
	glUniform1i(glGetUniformLocation(id, name), value);
}

void LightningEngine::Shader::SetFloat(const char* name, float value)
{
	ShaderStarted();
	glUniform1f(glGetUniformLocation(id, name), value);
}

void LightningEngine::Shader::SetV2(const char* name, V2 value)
{
	ShaderStarted();
	
	glUniform2f(glGetUniformLocation(id, name), value.x, value.y);
}

void LightningEngine::Shader::SetV3(const char* name, V3 value)
{
	ShaderStarted();
	
	glUniform3f(glGetUniformLocation(id, name), value.x, value.y, value.z);
}

void LightningEngine::Shader::SetLinearColor(const char* name, LinearColor value)
{
	ShaderStarted();
	
	glUniform3f(glGetUniformLocation(id, name), value.r, value.g, value.b);
}

void LightningEngine::Shader::SetColor(const char* name, C3 value)
{
	ShaderStarted();
	
	glUniform3f(glGetUniformLocation(id, name), value.r, value.g, value.b);
}

void LightningEngine::Shader::SetV4(const char* name, V4 value)
{
	ShaderStarted();
	
	glUniform4f(glGetUniformLocation(id, name), value.x, value.y, value.z, value.w);
}

void LightningEngine::Shader::SetMat2(const char* name, glm::mat2& m)
{
	ShaderStarted();
	
	glUniformMatrix2fv(glGetUniformLocation(id, name),1, GL_FALSE, &m[0][0]);
}

void LightningEngine::Shader::SetMat3(const char* name, glm::mat3& m)
{
	ShaderStarted();
	
	glUniformMatrix3fv(glGetUniformLocation(id, name), 1, GL_FALSE, &m[0][0]);
}

void LightningEngine::Shader::setMat4(const char* name, glm::mat4& m)
{
	ShaderStarted();

	glUniformMatrix4fv(glGetUniformLocation(id, name), 1, GL_FALSE, &m[0][0]);
}

const char* LightningEngine::Shader::MatrixChars(const char* ArrayName, int Position, const char* member)
{
	ShaderStarted();
	string an = ArrayName;
	string mb = member;
	string data = an + "[" + std::to_string(Position) + "]" + "." + mb;
	return data.c_str();
}

void LightningEngine::Shader::Projection()
{
	ShaderStarted();
	projection = glm::mat4(1.0f);
	setMat4("projection", projection);
}

void LightningEngine::Shader::View()
{
	ShaderStarted();
	
	view = glm::mat4(1.0f);
	setMat4("view", view);
}

void LightningEngine::Shader::Model()
{
	ShaderStarted();
	
	model = glm::mat4(1.0f);
	setMat4("model", model);
}


void LightningEngine::Shader::ShaderStarted()
{	
	//the use of warning avoids the constant call of the message
	if (!started && warning == false)
	{		
		Msg::Emit(Flow::WARNING_WNL, "Failed to initialize Shader -> ");
		cout << typeid(*this).name() << endl;
	}
	warning = true;
}

void LightningEngine::Shader::InitializeShader(ShaderSource _source)
{
	source = ShaderSource(_source);
	uint vertex, fragment;
	int success = 0;
	char infoLog[512] = { ' ' };
	//Vertex Shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &source.vertex, NULL);
	glCompileShader(vertex);
	ShaderMessageError(vertex, TypeProgram::VERTEX);
	//Fragment Shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &source.fragment, NULL);
	glCompileShader(fragment);
	ShaderMessageError(fragment, TypeProgram::FRAGMENT);
	//Shader Program
	id = glCreateProgram();
	glAttachShader(id, vertex);
	glAttachShader(id, fragment);
	glLinkProgram(id);
	ShaderMessageError(id, TypeProgram::PROGRAM);
	// exclua os shaders como eles estão vinculados ao nosso programa agora e não são mais necessários
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

void LightningEngine::Shader::ShaderMessageError(GLint shader, TypeProgram type)
{

	GLint success = 0;
	GLchar infoLog[1024] = { '0' };

	switch (type)
	{
	case LightningEngine::VERTEX:
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR TO COMPILE VERTEX SHADER " << type << "\n" << infoLog << "\n -- --------------------------- " << std::endl;
		}
		break;
	case LightningEngine::FRAGMENT:
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR TO COMPILE FRAGMENT SHADER " << type << "\n" << infoLog << "\n -- --------------------------- " << std::endl;
		}
		break;
	case LightningEngine::PROGRAM:
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR TO PROGRAM(S) LINKING " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
		break;
	case LightningEngine::GEOMETRY:
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERRO TO COMPILE GEOMETRY SHADER " << type << "\n" << infoLog << "\n -- --------------------------- " << std::endl;
		}
		break;
	}
}

void LightningEngine::LoadShader(Shader& shader, const char* vertex_path, const char* fragment_path, const char* geometry_path)
{
	const char* vertex = ReadFile(vertex_path, "VERTEX SHADER");
	const char* fragment = ReadFile(fragment_path, "FRAGMENT SHADER");

	

}

void LightningEngine::LoadShader(ShaderSource& source, const char* vertex_path, const char* fragment_path, const char* geometry_path)
{
	
}
