#pragma once

#include "math/Vec3.hpp"
#include "skeleton/BodyPart.hpp"

#include <array>

// Size of each part: x = width, y = length along the bone, z = depth.
// The skeleton derives every joint position from these values, so editing a
// size automatically moves all the parts attached to it.
class BodyDimensions
{
public:
	BodyDimensions();	// human proportions, about 1.9 units tall

	const Vec3& size(BodyPart part) const;
	void setSize(BodyPart part, const Vec3& size);	// components clamped to MIN_SIZE

	static constexpr float MIN_SIZE = 0.01f;

private:
	std::array<Vec3, BODY_PART_COUNT> sizes_;
};
