#pragma once

#include "math/Mat4.hpp"
#include "skeleton/BodyPart.hpp"

// Receives one call per body part. The skeleton does not know how (or if) a
// part is rendered: OpenGL, shadows, bounding-box queries and unit tests all
// plug in here (dependency inversion).
class IPartDrawer
{
public:
	virtual ~IPartDrawer() = default;

	// Draws a 1x1x1 shape centred on the origin of `model` (the current matrix).
	virtual void drawUnitShape(BodyPart part, const Mat4& model) = 0;
};
