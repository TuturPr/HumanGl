#pragma once

struct GLFWwindow;

// Owns the Dear ImGui context and its GLFW / OpenGL 3 backends.
// Must be created after the window's own input callbacks are installed:
// the GLFW backend chains to them.
class ImGuiLayer
{
public:
	explicit ImGuiLayer(GLFWwindow* window);
	~ImGuiLayer();

	ImGuiLayer(const ImGuiLayer&) = delete;
	ImGuiLayer& operator=(const ImGuiLayer&) = delete;

	void beginFrame();
	void endFrame();	// renders the widgets on top of the scene

	bool wantsMouse() const;
	bool wantsKeyboard() const;
};
