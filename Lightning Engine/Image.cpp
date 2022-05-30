#include "Image.h"

Lightning::Image::Image()
{

}

void Lightning::Image::Import(const char* vertex_path, const char* fragment_path, ShaderSource* Handler)
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

		string vs = vertexStream.str();
		Handler->vertex = vs.c_str();
		string fs = fragmentStream.str();
		Handler->fragment = fs.c_str();
		vertexSource = "";
		fragmentSource = "";
	}
	catch (std::ifstream::failure fail)
	{
		string ver_path = vertex_path, frag_path = fragment_path, spc = " ", msg_error;
		msg_error = vertex_path + spc + vertex_path + spc;
		Msg::Emit(Flow::PRINT, "Error to import shader" + msg_error);
		vertexSource = "";
		fragmentSource = "";
	}
}

Lightning::uint Lightning::Image::TextureFromFile(const char* path, const string& directory, bool gamma)
{
	string filename = string(path);
	if (directory != " ")
		filename = directory + '/' + filename;
	else
		filename = "";

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
		string msg_error = path;
		Msg::Emit(Flow::PRINT, "Texture failed to load at path: " + msg_error);
		stbi_image_free(data);
	}

	return textureID;
}

Lightning::uint Lightning::Image::TextureFromFile(const char* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
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
		string msg_error = path;
		Msg::Emit(Flow::PRINT, "Texture failed to load at path: " + msg_error);
		stbi_image_free(data);
	}

	return textureID;
}
