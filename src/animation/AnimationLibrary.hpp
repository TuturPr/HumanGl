#pragma once

#include "animation/IAnimation.hpp"

#include <memory>
#include <vector>

// Owns every available animation. New moves are added by registering them,
// without touching the animator, the renderer or the UI (open/closed).
class AnimationLibrary
{
public:
	static AnimationLibrary createDefault();

	void add(std::unique_ptr<IAnimation> animation);
	std::size_t size() const;
	const IAnimation& at(std::size_t index) const;
	const IAnimation* find(const std::string& name) const;

private:
	std::vector<std::unique_ptr<IAnimation>> animations_;
};
