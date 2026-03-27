#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Mesh.hpp"

GLFWwindow* initGLFW();
void loadModel(const std::string& path, Mesh& mesh, GLuint& textureID);
void handleTextureChange(Mesh& mesh, GLuint& textureID);
