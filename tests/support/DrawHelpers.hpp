#pragma once

#include <gtest/gtest.h>

#include "math/MatrixStack.hpp"
#include "skeleton/Character.hpp"
#include "skeleton/HumanModel.hpp"

#include <algorithm>
#include <array>
#include <limits>
#include <vector>

namespace testing_support {

// Stores every drawUnitShape call (part + matrix).
class RecordingDrawer : public IPartDrawer
{
public:
	struct Call
	{
		BodyPart part;
		Mat4 matrix;
	};

	void drawUnitShape(BodyPart part, const Mat4& model) override { calls.push_back({part, model}); }

	int count(BodyPart part) const
	{
		return static_cast<int>(std::count_if(calls.begin(), calls.end(),
			[part](const Call& c) { return c.part == part; }));
	}

	// Matrix of the (single) call for `part`; fails the test when missing.
	const Mat4& matrix(BodyPart part) const
	{
		for (const Call& c : calls)
			if (c.part == part)
				return c.matrix;
		ADD_FAILURE() << "part not drawn: " << bodyPartName(part);
		static const Mat4 fallback;
		return fallback;
	}

	Vec3 center(BodyPart part) const { return matrix(part).transformPoint({0, 0, 0}); }

	std::vector<Call> calls;
};

inline std::array<Vec3, 8> cubeCorners()
{
	std::array<Vec3, 8> corners;
	std::size_t i = 0;
	for (float x : {-0.5f, 0.5f})
		for (float y : {-0.5f, 0.5f})
			for (float z : {-0.5f, 0.5f})
				corners[i++] = {x, y, z};
	return corners;
}

// Tracks the lowest world-space y of the unit-cube corners of all drawn parts.
class LowestPointDrawer : public IPartDrawer
{
public:
	void drawUnitShape(BodyPart, const Mat4& model) override
	{
		for (const Vec3& corner : cubeCorners())
			lowest = std::min(lowest, model.transformPoint(corner).y);
	}

	float lowest = std::numeric_limits<float>::max();
};

inline RecordingDrawer drawModel(const HumanModel& model, const Pose& pose)
{
	MatrixStack stack;
	RecordingDrawer drawer;
	model.draw(stack, pose, drawer);
	return drawer;
}

inline float lowestDrawnY(const Character& character, const Pose& pose)
{
	MatrixStack stack;
	LowestPointDrawer drawer;
	character.draw(stack, pose, drawer);
	return drawer.lowest;
}

inline void expectVecNear(const Vec3& a, const Vec3& b, float eps = 1e-4f)
{
	EXPECT_NEAR(a.x, b.x, eps);
	EXPECT_NEAR(a.y, b.y, eps);
	EXPECT_NEAR(a.z, b.z, eps);
}

inline bool sameMatrix(const Mat4& a, const Mat4& b, float eps = 1e-5f)
{
	for (int i = 0; i < 16; ++i)
		if (std::abs(a.data()[i] - b.data()[i]) > eps)
			return false;
	return true;
}

} // namespace testing_support
