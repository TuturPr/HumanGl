#pragma once

#include "math/Vec3.hpp"
#include "skeleton/BodyPart.hpp"

#include <array>

// Colour of each body part (sRGB, components in [0, 1]).
class BodyPalette
{
public:
	BodyPalette();

	const Vec3& color(BodyPart part) const;
	void setColor(BodyPart part, const Vec3& color);

private:
	std::array<Vec3, BODY_PART_COUNT> colors_;
};
