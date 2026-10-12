#include "animation/Animator.hpp"

#include "math/MathUtils.hpp"

#include <algorithm>
#include <cmath>

namespace {

// Same orientation as `angle`, but within 180 degrees of `reference`, so that
// a cross-fade turns the short way round.
float nearestEquivalentAngle(float angle, float reference)
{
	return reference + std::remainder(angle - reference, 360.0f);
}

} // namespace

Animator::Animator(JointLimits limits) : limits_(limits) {}

void Animator::play(const IAnimation& animation)
{
	if (current_ == &animation)
		return;
	fadeFrom_ = pose_;
	fadeElapsed_ = 0.0f;
	current_ = &animation;
	time_ = 0.0f;
}

void Animator::update(float deltaSeconds)
{
	if (!current_)
		return;
	if (!paused_)
	{
		time_ += deltaSeconds * speed_;
		fadeElapsed_ += deltaSeconds;
	}

	Pose target = sampleCurrent();
	if (fadeElapsed_ < fadeDuration_)
	{
		Pose from = fadeFrom_;
		from.rootYaw = nearestEquivalentAngle(from.rootYaw, target.rootYaw);
		target = lerp(from, target, math::smoothstep(fadeElapsed_ / fadeDuration_));
	}
	pose_ = limits_.clamp(target);
}

Pose Animator::sampleCurrent() const
{
	return current_ ? current_->sample(time_) : Pose{};
}

const Pose& Animator::pose() const
{
	return pose_;
}

const IAnimation* Animator::current() const
{
	return current_;
}

float Animator::speed() const
{
	return speed_;
}

void Animator::setSpeed(float speed)
{
	speed_ = std::max(speed, 0.0f);
}

bool Animator::paused() const
{
	return paused_;
}

void Animator::setPaused(bool paused)
{
	paused_ = paused;
}

float Animator::transitionDuration() const
{
	return fadeDuration_;
}

void Animator::setTransitionDuration(float seconds)
{
	fadeDuration_ = std::max(seconds, 0.0f);
}
