#pragma once
#include<glad/glad.h>
#include<string>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

std::string get_file_contents(const char * filename);

class Shader {
	public:
		GLuint ID;
		Shader(const char* vertexFile, const char* fragmentFile);

		void Activate();
		void Delete();
		void setMat4(const char* name, glm::mat4 value);
		void setInt(const char* name, int value);

		void compileErrors(unsigned int shader, const char *type);
};
