#include "headers/app_callbacks.hpp"
#include "headers/app_state.hpp"
#include "headers/Camera.hpp"
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height){
	windowWidth = (float)width;
	windowHeight = (float)height;

	float currentAspectRatio = (float)width / (float)height;
	int viewportWidth, viewportHeight, viewportX = 0, viewportY = 0;

	if (currentAspectRatio > BASE_ASPECT_RATIO) {
		viewportHeight = height;
		viewportWidth = (int)(height * BASE_ASPECT_RATIO);
		viewportX = (width - viewportWidth) / 2;
	} else {
		viewportWidth = width;
		viewportHeight = (int)(width / BASE_ASPECT_RATIO);
		viewportY = (height - viewportHeight) / 2;
	}

	glViewport(viewportX, viewportY, viewportWidth, viewportHeight);
	(void) window;
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// Toggle mouse control with M
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
        if (!keyPressed_M) {
            mouseControlEnabled = !mouseControlEnabled;
            std::cout << "Mouse Look: " << (mouseControlEnabled ? "ENABLED" : "DISABLED") << std::endl;

            // Lock/unlock cursor based on mouselook state
            if (mouseControlEnabled) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                // Reset mouse position to center
                glfwSetCursorPos(window, windowWidth / 2.0, windowHeight / 2.0);
                firstMouse = true;  // Reset flag to avoid jump
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }

            keyPressed_M = true;
        }
    } else {
        keyPressed_M = false;
    }

	// Toggle vertex mode with V
	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
		if (!keyPressed_V) {
			vertexMode = !vertexMode;
			std::cout << "Vertex Mode: " << (vertexMode ? "ON" : "OFF") << std::endl;
			keyPressed_V = true;
		}
	} else {
		keyPressed_V = false;
	}

	// Camera movement (always available)
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, deltaTime);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	if (!mouseControlEnabled)
		return;

	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
	(void)window;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
	(void)window;
	(void)xoffset;
}
