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
#include "headers/Animation.hpp"
#include "headers/Animations.hpp"

int main(void) {
	GLFWwindow* window = initGLFW();
	if (!window)
		return -1;

	Model entityModel;
	Shader shaderProgram("src/shaders/default.vert", "src/shaders/default.frag");
	shaderProgram.Activate();

	GLuint textureID = Texture::createGrayTexture();
	shaderProgram.setInt("ourTexture", 0);

	while (!glfwWindowShouldClose(window)) {
		// Update timing FIRST
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Input and events
		processInput(window);
		glfwPollEvents();

		// Clear screen
		glClearColor(0.03f, 0.025f, 0.02f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Set up transformation matrices
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), BASE_ASPECT_RATIO, 0.1f, 100.0f);

		// Set polygon mode (wireframe or solid)
		glPolygonMode(GL_FRONT_AND_BACK, vertexMode ? GL_LINE : GL_FILL);

		// Render
		entityModel.mesh.setTextureId(0, textureID);
		shaderProgram.Activate();
		shaderProgram.setMat4("view", view);
		shaderProgram.setMat4("projection", projection);

		if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		    entityModel.setAnimation(Animations::idle());
		if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		    entityModel.setAnimation(Animations::walk());
		if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
		    entityModel.setAnimation(Animations::jump());
		if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
		    entityModel.setAnimation(Animations::disco());

		entityModel.updateAnimation(deltaTime);
		entityModel.DrawModel(shaderProgram);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwSwapInterval(1);
	}

	glfwTerminate();
	return 0;
}

