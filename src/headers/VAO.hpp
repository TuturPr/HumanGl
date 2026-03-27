#pragma once
#include "glad/glad.h"
#include "headers/VBO.hpp"

class VAO
{
	public:
		GLuint ID;
		VAO();

		void linkAttrib(VBO& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset);
		void Bind();
		void Unbind();
		void Delete();
};

