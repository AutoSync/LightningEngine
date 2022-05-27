#pragma once
#include "lightning-stream.h"

Lightning::Stream::Importer::Importer()
{
}

Lightning::Stream::Importer::~Importer()
{
}

void Lightning::Stream::Importer::Import(flag flags, const char* vertex_path, const char* fragment_path, Equinox::Handler* Handler)
{
	switch(flags)
	{
	case RI_GL:
		if (Handler->RenderHardwareInterface == Equinox::RHI::OpenGL)
		{
			std::string vertexSource, fragmentSource;
			std::fstream vertexFile, fragmentFile;

			vertexFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			fragmentFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			try
			{
				vertexFile.open(vertex_path);
				fragmentFile.open(fragment_path);

				std::stringstream vertexStream, fragmentStream;

				vertexStream << vertexFile.rdbuf();
				fragmentStream << fragmentFile.rdbuf();

				vertexFile.close();
				fragmentFile.close();

				Handler->VertexShader = vertexStream.str();
				Handler->FragmentShader = fragmentStream.str();
				vertexSource = "";
				fragmentSource = "";
			}
			catch (std::ifstream::failure fail)
			{
				std::cout << "ERROR::EQUINOX::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
				vertexSource = "";
				fragmentSource = "";
			}
		}
		else
		{
			std::cout << "ERROR::EQUINOX::SHADER::NOT_AN_OPENGL_SHADER" << std::endl;
			//MessageBox(glfwGetWin32Window(glfwGetCurrentContext()), L"Not an Opengl Shader", L"EQUINOX SHADER ERROR", MB_ICONSTOP);
		}
		break;
	}

}

uint Lightning::Stream::Importer::TextureFromFile(solidtext path, const text& directory, bool gamma)
{
	text filename = text(path);
	filename = directory + '/' + filename;

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format = 0;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}


