#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "headers/ShaderClass.hpp"
#include "headers/Texture.hpp"
#include "headers/Mesh.hpp"
#include "headers/app_state.hpp"
#include "headers/app_callbacks.hpp"
#include "headers/app_logic.hpp"
#include "headers/Model.hpp"
#include "headers/Matstack.hpp"

int main(void) {
	GLFWwindow* window = initGLFW();
	if (!window)
		return -1;

	Model entityModel;
	Matstack matstack;
	Shader shaderProgram("src/shaders/default.vert", "src/shaders/default.frag");
	shaderProgram.Activate();

	GLuint textureID = Texture::createGrayTexture();
	shaderProgram.setInt("ourTexture", 0);

	while (!glfwWindowShouldClose(window)) {
		// Input and events
		processInput(window);
		glfwPollEvents();

		// Clear screen
		glClearColor(0.03f, 0.025f, 0.02f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Set up transformation matrices
		glm::mat4 model = matstack.stack.back();
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), BASE_ASPECT_RATIO, 0.1f, 100.0f);

		// Set polygon mode (wireframe or solid)
		glPolygonMode(GL_FRONT_AND_BACK, vertexMode ? GL_LINE : GL_FILL);

		// Render
		entityModel.mesh.setTextureId(0, textureID);
		shaderProgram.Activate();
		shaderProgram.setMat4("view", view);
		shaderProgram.setMat4("projection", projection);

		matstack.pushMatrix();
		matstack.scale(glm::vec3(0.5f, 2.0f, 0.5f));
		matstack.translate(glm::vec3(1.0f, 0.0f, -0.7f));
		//matstack.rotate((float)glfwGetTime() * 50.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		model = matstack.stack.back();
		entityModel.DrawCube(shaderProgram, model);
		matstack.popMatrix();
		
		// Swap buffers and update timing
		glfwSwapBuffers(window);
		glfwSwapInterval(1);

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
	}

	glfwTerminate();
	return 0;
}

