#include "headers/app_logic.hpp"
#include "headers/app_state.hpp"
#include "headers/Texture.hpp"
#include "headers/ObjLoader.hpp"
#include "headers/ShaderClass.hpp"
#include "headers/Camera.hpp"
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

GLFWwindow* initGLFW()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(1280, 720, "Scop", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return nullptr;
	}

	extern void framebuffer_size_callback(GLFWwindow*, int, int);
	extern void mouse_callback(GLFWwindow*, double, double);
	extern void scroll_callback(GLFWwindow*, double, double);

	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return nullptr;
	}

	glViewport(0, 0, (int)windowWidth, (int)windowHeight);
	glEnable(GL_DEPTH_TEST);

	return window;
}

void loadModel(const std::string& path, Mesh& mesh, GLuint& textureID)
{
	std::cout << "Loading model: " << path << "\n";
	try {
		ObjLoader loader;
		mesh = loader.load(path);

		if (!mesh.textures.empty() && !mesh.textures[0].path.empty()) {
			try {
				Texture tex(mesh.textures[0].path.c_str(), GL_TEXTURE_2D, GL_TEXTURE0, GL_RGB, GL_UNSIGNED_BYTE);
				textureID = tex.ID;
				mesh.setTextureId(0, textureID);
				std::cout << "Loaded MTL texture: " << mesh.textures[0].path << "\n";
			} catch (const std::exception& e) {
				std::cout << "Failed to load MTL texture: " << e.what() << "\n";
				textureID = Texture::createGrayTexture();
				mesh.addTexture(textureID, "diffuse");
			}
		} else {
			textureID = Texture::createGrayTexture();
			mesh.addTexture(textureID, "diffuse");
			std::cout << "Using gray texture for model\n";
		}
	} catch (const std::exception& e) {
		std::cout << "Error loading model: " << e.what() << "\n";
	}
}

void handleTextureChange(Mesh& mesh, GLuint& textureID)
{
	// Placeholder for texture change handling
	std::cout << "Texture change handling not yet implemented\n";
	(void)mesh;
	(void)textureID;
}
