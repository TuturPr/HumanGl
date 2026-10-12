#pragma once

#include "animation/AnimationLibrary.hpp"
#include "animation/Animator.hpp"
#include "render/OrbitCamera.hpp"
#include "render/SceneRenderer.hpp"
#include "skeleton/Character.hpp"

// The ImGui window: switches animations and edits the body at runtime.
// It only edits the objects it is given; it owns nothing.
class ControlPanel
{
public:
	ControlPanel(const AnimationLibrary& animations, Animator& animator, Character& character,
				 SceneRenderer& renderer, OrbitCamera& camera);

	void draw();

private:
	void drawAnimationSection();
	void drawBodySection();
	void drawDisplaySection();
	void drawHelpSection();

	const AnimationLibrary& animations_;
	Animator& animator_;
	Character& character_;
	SceneRenderer& renderer_;
	OrbitCamera& camera_;

	BodyPart selectedPart_ = BodyPart::UpperArmLeft;
	bool mirrorEdits_ = true;
};
