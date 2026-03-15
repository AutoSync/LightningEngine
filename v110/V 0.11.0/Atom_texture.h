#ifndef LIGHTNING_LOAD_IMPORTER_H
#define LIGHTNING_LOAD_IMPORTER_H

#include <glad/glad.h> // Gerenciador de Ponteiros
#include <GLFW/glfw3.h> // API do OpenGL
#include "stb_image.h"//Image Importer

#include <iostream>

class LightningImporter
{
public:
	unsigned int Texture(const char* Path)
	{
		static unsigned int TextureID;
		glGenTextures(1, &TextureID);
		
		static int width, height, numberComponents;
		static unsigned char* Data = stbi_load(Path, &width, &height, &numberComponents,0);
		if (Data)
		{
			std::cout << numberComponents<<std::endl;
			GLenum format;
			if (numberComponents == 1)
				format = GL_RED; // GL_RED
			else if (numberComponents == 3)
				format = GL_RGB;
			else if (numberComponents == 4)
				format = GL_RGBA;
			
			glBindTexture(GL_TEXTURE_2D, TextureID);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, Data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(Data);
		}
		else
		{
			std::cout << "Texture failed to load at path: " << Path << std::endl;
			stbi_image_free(Data);
		}
		return TextureID;
	}
};


#endif