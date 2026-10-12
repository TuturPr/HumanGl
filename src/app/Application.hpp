#pragma once

#include "animation/AnimationLibrary.hpp"
#include "animation/Animator.hpp"
#include "platform/Window.hpp"
#include "render/OrbitCamera.hpp"
#include "render/SceneRenderer.hpp"
#include "skeleton/Character.hpp"
#include "ui/ControlPanel.hpp"
#include "ui/ImGuiLayer.hpp"

#include <string>

// Wires the independent modules together and runs the main loop:
// input -> animator (pose) -> renderer (character + pose) -> UI on top.
class Application
{
public:
	explicit Application(const std::string& shaderDirectory);
	void run();

private:
	void handleInput();
	void handleKeyboard();
	void handleMouse();

	Window window_;
	MeshData unitShape_;
	Character character_;
	SceneRenderer renderer_;
	AnimationLibrary animations_;
	Animator animator_;
	OrbitCamera camera_;
	ImGuiLayer imgui_;
	ControlPanel panel_;
};
