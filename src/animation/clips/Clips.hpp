#pragma once

#include "animation/IAnimation.hpp"

#include <memory>

// Factories of the built-in animations. Each one lives in its own file.
namespace clips {

std::unique_ptr<IAnimation> createIdle();
std::unique_ptr<IAnimation> createWalk();
std::unique_ptr<IAnimation> createJump();

} // namespace clips
