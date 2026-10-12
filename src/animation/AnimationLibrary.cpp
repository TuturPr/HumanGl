#include "animation/AnimationLibrary.hpp"

#include "animation/clips/Clips.hpp"

AnimationLibrary AnimationLibrary::createDefault()
{
	AnimationLibrary library;
	library.add(clips::createIdle());
	library.add(clips::createWalk());
	library.add(clips::createJump());
	return library;
}

void AnimationLibrary::add(std::unique_ptr<IAnimation> animation)
{
	animations_.push_back(std::move(animation));
}

std::size_t AnimationLibrary::size() const
{
	return animations_.size();
}

const IAnimation& AnimationLibrary::at(std::size_t index) const
{
	return *animations_.at(index);
}

const IAnimation* AnimationLibrary::find(const std::string& name) const
{
	for (const auto& animation : animations_)
		if (animation->name() == name)
			return animation.get();
	return nullptr;
}
