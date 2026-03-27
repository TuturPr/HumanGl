#include "headers/Texture.hpp"

#include <cstdio>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

namespace {
	std::vector<unsigned char> loadImage(const char* image, int& width, int& height, int& channels, bool flipY = false)
	{
		stbi_set_flip_vertically_on_load(flipY);

		unsigned char* data = stbi_load(image, &width, &height, &channels, 4);
		if (!data)
			throw std::runtime_error(std::string("Failed to load image: ") + image);

		size_t size = static_cast<size_t>(width) * static_cast<size_t>(height) * 4;
		std::vector<unsigned char> pixels(data, data + size);

		stbi_image_free(data);

		channels = 4;
		return pixels;
	}
}

Texture::Texture(const char *image, GLenum texType, GLenum slot, GLenum format, GLenum pixelType) {
	type = texType;
	int widthImg = 0;
	int heightImg = 0;
	int numColCh = 0;

	std::vector<unsigned char> bytes = loadImage(image, widthImg, heightImg, numColCh, true);

	glGenTextures(1, &ID);
	glActiveTexture(slot);
	glBindTexture(texType, ID);

	glTexParameteri(texType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(texType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(texType, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(texType, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if (numColCh == 3)
		format = GL_RGB;
	else if (numColCh == 1)
		format = GL_RED;
	else if (numColCh == 4)
		format = GL_RGBA;

	glTexImage2D(texType, 0, format, widthImg, heightImg, 0, format, pixelType, bytes.data());
	glGenerateMipmap(texType);

	glBindTexture(texType, 0);

}

Texture::Texture(const char *image, GLenum texType, GLenum slot, GLenum format, GLenum pixelType, bool flipPNG) {
	type = texType;
	int widthImg = 0;
	int heightImg = 0;
	int numColCh = 0;

	std::vector<unsigned char> bytes = loadImage(image, widthImg, heightImg, numColCh, flipPNG);

	glGenTextures(1, &ID);
	glActiveTexture(slot);
	glBindTexture(texType, ID);

	glTexParameteri(texType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(texType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(texType, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(texType, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if (numColCh == 3)
		format = GL_RGB;
	else if (numColCh == 1)
		format = GL_RED;
	else if (numColCh == 4)
		format = GL_RGBA;

	glTexImage2D(texType, 0, format, widthImg, heightImg, 0, format, pixelType, bytes.data());
	glGenerateMipmap(texType);

	glBindTexture(texType, 0);

}

void Texture::texUnit(Shader& shader, const char *uniform, GLuint unit) {
	GLuint tex0Uni = glGetUniformLocation(shader.ID, uniform);
	shader.Activate();
	glUniform1i(tex0Uni, unit);
}

void Texture::Bind() {
	glBindTexture(type, ID);
}

void Texture::Unbind() {
	glBindTexture(type, 0);
}

void Texture::Delete() {
	glDeleteTextures(1, &ID);
}

GLuint Texture::createWhiteTexture() {
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	unsigned char whitePixel[] = {255, 255, 255, 255};
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, 0);
	return textureID;
}

GLuint Texture::createGrayTexture() {
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	unsigned char grayPixel[] = {128, 128, 128, 255};
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, grayPixel);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, 0);
	return textureID;
}
