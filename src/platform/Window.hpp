#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <vector>

// GLFW window with an OpenGL 4.1 core context. Collects the input events of
// the frame so that the application can poll them.
class Window
{
public:
	Window(int width, int height, const std::string& title);
	~Window();

	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;

	bool shouldClose() const;
	void close();
	void swapBuffers();
	void pollEvents();	// clears the previous frame's events first

	GLFWwindow* handle() const;
	void framebufferSize(int& width, int& height) const;
	double time() const;

	// Events of the current frame.
	const std::vector<int>& pressedKeys() const;
	double scrollDelta() const;
	bool isMouseButtonDown(int button) const;
	void cursorDelta(double& dx, double& dy) const;

private:
	static void onKey(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void onScroll(GLFWwindow* window, double xOffset, double yOffset);

	GLFWwindow* window_ = nullptr;
	std::vector<int> pressedKeys_;
	double scrollDelta_ = 0.0;
	double cursorX_ = 0.0;
	double cursorY_ = 0.0;
	double previousCursorX_ = 0.0;
	double previousCursorY_ = 0.0;
};
