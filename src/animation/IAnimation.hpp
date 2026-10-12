#pragma once

#include "skeleton/Pose.hpp"

#include <string>

// An animation is a pure function of time: it only produces poses and knows
// nothing about rendering, OpenGL or body dimensions.
class IAnimation
{
public:
	virtual ~IAnimation() = default;

	virtual const std::string& name() const = 0;
	// Pose at `time` seconds since the animation started; animations loop.
	virtual Pose sample(float time) const = 0;
};
