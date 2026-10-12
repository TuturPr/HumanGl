#pragma once

#include "math/Mat4.hpp"

#include <glad/glad.h>
#include <string>

// Compiled and linked GLSL program loaded from two source files.
// Throws std::runtime_error with the GL info log on failure.
class ShaderProgram
{
public:
	ShaderProgram(const std::string& vertexPath, const std::string& fragmentPath);
	~ShaderProgram();

	ShaderProgram(const ShaderProgram&) = delete;
	ShaderProgram& operator=(const ShaderProgram&) = delete;

	void use() const;
	void setMat4(const char* name, const Mat4& value) const;
	void setVec3(const char* name, const Vec3& value) const;
	void setFloat(const char* name, float value) const;

private:
	GLint location(const char* name) const;

	GLuint id_ = 0;
};
