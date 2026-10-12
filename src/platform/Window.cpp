#include "platform/Window.hpp"

#include <stdexcept>

Window::Window(int width, int height, const std::string& title)
{
	if (!glfwInit())
		throw std::runtime_error("Failed to initialise GLFW");

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_STENCIL_BITS, 8);
	glfwWindowHint(GLFW_SAMPLES, 4);

	window_ = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	if (!window_)
	{
		glfwTerminate();
		throw std::runtime_error("Failed to create an OpenGL 4.1 window");
	}
	glfwMakeContextCurrent(window_);
	glfwSwapInterval(1);

	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		glfwDestroyWindow(window_);
		glfwTerminate();
		throw std::runtime_error("Failed to load OpenGL functions");
	}
	glEnable(GL_MULTISAMPLE);

	glfwSetWindowUserPointer(window_, this);
	glfwSetKeyCallback(window_, onKey);
	glfwSetScrollCallback(window_, onScroll);
	glfwGetCursorPos(window_, &cursorX_, &cursorY_);
	previousCursorX_ = cursorX_;
	previousCursorY_ = cursorY_;
}

Window::~Window()
{
	glfwDestroyWindow(window_);
	glfwTerminate();
}

bool Window::shouldClose() const
{
	return glfwWindowShouldClose(window_);
}

void Window::close()
{
	glfwSetWindowShouldClose(window_, GLFW_TRUE);
}

void Window::swapBuffers()
{
	glfwSwapBuffers(window_);
}

void Window::pollEvents()
{
	pressedKeys_.clear();
	scrollDelta_ = 0.0;
	previousCursorX_ = cursorX_;
	previousCursorY_ = cursorY_;
	glfwPollEvents();
	glfwGetCursorPos(window_, &cursorX_, &cursorY_);
}

GLFWwindow* Window::handle() const
{
	return window_;
}

void Window::framebufferSize(int& width, int& height) const
{
	glfwGetFramebufferSize(window_, &width, &height);
}

double Window::time() const
{
	return glfwGetTime();
}

const std::vector<int>& Window::pressedKeys() const
{
	return pressedKeys_;
}

double Window::scrollDelta() const
{
	return scrollDelta_;
}

bool Window::isMouseButtonDown(int button) const
{
	return glfwGetMouseButton(window_, button) == GLFW_PRESS;
}

void Window::cursorDelta(double& dx, double& dy) const
{
	dx = cursorX_ - previousCursorX_;
	dy = cursorY_ - previousCursorY_;
}

void Window::onKey(GLFWwindow* window, int key, int, int action, int)
{
	if (action == GLFW_PRESS)
		static_cast<Window*>(glfwGetWindowUserPointer(window))->pressedKeys_.push_back(key);
}

void Window::onScroll(GLFWwindow* window, double, double yOffset)
{
	static_cast<Window*>(glfwGetWindowUserPointer(window))->scrollDelta_ += yOffset;
}
