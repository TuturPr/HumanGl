#pragma once

#include "animation/IAnimation.hpp"
#include "skeleton/JointLimits.hpp"

// Plays one animation at a time and produces the pose to display.
//  - switching animation cross-fades from the current pose (no popping)
//  - speed and pause only affect the animation clock
//  - every output pose is clamped to the joint limits, so no animation can
//    ever bend a joint the wrong way
class Animator
{
public:
	explicit Animator(JointLimits limits = {});

	void play(const IAnimation& animation);
	void update(float deltaSeconds);

	const Pose& pose() const;
	const IAnimation* current() const;

	float speed() const;
	void setSpeed(float speed);	// negative values are clamped to 0
	bool paused() const;
	void setPaused(bool paused);
	float transitionDuration() const;
	void setTransitionDuration(float seconds);

private:
	Pose sampleCurrent() const;

	JointLimits limits_;
	const IAnimation* current_ = nullptr;
	float time_ = 0.0f;
	float speed_ = 1.0f;
	bool paused_ = false;

	Pose pose_;
	Pose fadeFrom_;
	float fadeElapsed_ = 0.0f;
	float fadeDuration_ = 0.4f;
};
