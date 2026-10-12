#pragma once

#include "animation/IAnimation.hpp"

#include <functional>
#include <string>
#include <utility>

namespace testing_support {

// Animation defined by a lambda: lets a test script the exact pose it needs.
class FakeAnimation : public IAnimation
{
public:
	FakeAnimation(std::string name, std::function<Pose(float)> fn)
		: name_(std::move(name)), fn_(std::move(fn)) {}

	const std::string& name() const override { return name_; }
	Pose sample(float time) const override { return fn_(time); }

private:
	std::string name_;
	std::function<Pose(float)> fn_;
};

} // namespace testing_support
