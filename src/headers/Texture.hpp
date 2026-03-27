#pragma once

#include "glad/glad.h"
#include "headers/ShaderClass.hpp"

class Texture {
	public:
		GLuint ID;
		GLenum type;
		Texture(const char* image, GLenum texType, GLenum slot, GLenum format, GLenum pixelType);
		Texture(const char* image, GLenum texType, GLenum slot, GLenum format, GLenum pixelType, bool flipPNG);
		static GLuint createWhiteTexture();
		static GLuint createGrayTexture();

		void texUnit(Shader& shader, const char* uniform, GLuint unit);
		void Bind();
		void Unbind();
		void Delete();
};
