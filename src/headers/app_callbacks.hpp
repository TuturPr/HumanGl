#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <functional>

// Callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// Input processing
void processInput(GLFWwindow *window);

// Utilities
void handleToggleKey(int key, GLFWwindow* window, bool& keyPressed, std::function<void()> action);
void CalculateFrameRate();
