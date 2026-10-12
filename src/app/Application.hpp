#pragma once

#include "animation/AnimationLibrary.hpp"
#include "animation/Animator.hpp"
#include "platform/Window.hpp"
#include "render/OrbitCamera.hpp"
#include "render/SceneRenderer.hpp"
#include "skeleton/Character.hpp"

#include <string>

// Wires the independent modules together and runs the main loop:
// input -> animator (pose) -> renderer (character + pose).
class Application
{
public:
	explicit Application(const std::string& shaderDirectory);
	void run();

private:
	void handleInput();

	Window window_;
	MeshData unitShape_;
	Character character_;
	SceneRenderer renderer_;
	AnimationLibrary animations_;
	Animator animator_;
	OrbitCamera camera_;
};
