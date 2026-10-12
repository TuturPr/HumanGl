#include "render/ShaderProgram.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace {

std::string readFile(const std::string& path)
{
	std::ifstream file(path);
	if (!file)
		throw std::runtime_error("Cannot open shader file: " + path);
	std::stringstream content;
	content << file.rdbuf();
	return content.str();
}

GLuint compile(GLenum type, const std::string& path)
{
	const std::string source = readFile(path);
	const char* text = source.c_str();
	const GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &text, nullptr);
	glCompileShader(shader);

	GLint ok = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
	if (ok != GL_TRUE)
	{
		char log[1024];
		glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
		glDeleteShader(shader);
		throw std::runtime_error("Shader compilation failed (" + path + "):\n" + log);
	}
	return shader;
}

} // namespace

ShaderProgram::ShaderProgram(const std::string& vertexPath, const std::string& fragmentPath)
{
	const GLuint vertex = compile(GL_VERTEX_SHADER, vertexPath);
	const GLuint fragment = compile(GL_FRAGMENT_SHADER, fragmentPath);

	id_ = glCreateProgram();
	glAttachShader(id_, vertex);
	glAttachShader(id_, fragment);
	glLinkProgram(id_);
	glDeleteShader(vertex);
	glDeleteShader(fragment);

	GLint ok = GL_FALSE;
	glGetProgramiv(id_, GL_LINK_STATUS, &ok);
	if (ok != GL_TRUE)
	{
		char log[1024];
		glGetProgramInfoLog(id_, sizeof(log), nullptr, log);
		glDeleteProgram(id_);
		throw std::runtime_error(std::string("Shader link failed:\n") + log);
	}
}

ShaderProgram::~ShaderProgram()
{
	glDeleteProgram(id_);
}

void ShaderProgram::use() const
{
	glUseProgram(id_);
}

GLint ShaderProgram::location(const char* name) const
{
	return glGetUniformLocation(id_, name);
}

void ShaderProgram::setMat4(const char* name, const Mat4& value) const
{
	glUniformMatrix4fv(location(name), 1, GL_FALSE, value.data());
}

void ShaderProgram::setVec3(const char* name, const Vec3& value) const
{
	glUniform3f(location(name), value.x, value.y, value.z);
}

void ShaderProgram::setFloat(const char* name, float value) const
{
	glUniform1f(location(name), value);
}
