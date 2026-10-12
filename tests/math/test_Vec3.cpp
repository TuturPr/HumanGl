#include <gtest/gtest.h>

#include "math/MathUtils.hpp"
#include "math/Vec3.hpp"

namespace {
constexpr float EPS = 1e-5f;

void expectVec(const Vec3& a, const Vec3& b)
{
	EXPECT_NEAR(a.x, b.x, EPS);
	EXPECT_NEAR(a.y, b.y, EPS);
	EXPECT_NEAR(a.z, b.z, EPS);
}
}

TEST(Vec3, DefaultIsZero) { expectVec(Vec3(), Vec3(0, 0, 0)); }
TEST(Vec3, Addition) { expectVec(Vec3(1, 2, 3) + Vec3(4, 5, 6), Vec3(5, 7, 9)); }
TEST(Vec3, Subtraction) { expectVec(Vec3(4, 5, 6) - Vec3(1, 2, 3), Vec3(3, 3, 3)); }
TEST(Vec3, Negation) { expectVec(-Vec3(1, -2, 3), Vec3(-1, 2, -3)); }
TEST(Vec3, ScalarMultiplicationBothSides)
{
	expectVec(Vec3(1, 2, 3) * 2.0f, Vec3(2, 4, 6));
	expectVec(2.0f * Vec3(1, 2, 3), Vec3(2, 4, 6));
}
TEST(Vec3, ScalarDivision) { expectVec(Vec3(2, 4, 6) / 2.0f, Vec3(1, 2, 3)); }
TEST(Vec3, CompoundAssignment)
{
	Vec3 v(1, 1, 1);
	v += Vec3(1, 2, 3);
	expectVec(v, Vec3(2, 3, 4));
	v -= Vec3(1, 1, 1);
	expectVec(v, Vec3(1, 2, 3));
	v *= 3.0f;
	expectVec(v, Vec3(3, 6, 9));
}
TEST(Vec3, DotProduct) { EXPECT_NEAR(Vec3(1, 2, 3).dot(Vec3(4, -5, 6)), 12.0f, EPS); }
TEST(Vec3, DotOfOrthogonalIsZero) { EXPECT_NEAR(Vec3(1, 0, 0).dot(Vec3(0, 1, 0)), 0.0f, EPS); }
TEST(Vec3, CrossXYIsZ) { expectVec(Vec3(1, 0, 0).cross(Vec3(0, 1, 0)), Vec3(0, 0, 1)); }
TEST(Vec3, CrossIsAntiCommutative)
{
	Vec3 a(1, 2, 3), b(-2, 0.5f, 4);
	expectVec(a.cross(b), -b.cross(a));
}
TEST(Vec3, CrossIsPerpendicularToOperands)
{
	Vec3 a(1, 2, 3), b(-2, 0.5f, 4);
	Vec3 c = a.cross(b);
	EXPECT_NEAR(c.dot(a), 0.0f, 1e-4f);
	EXPECT_NEAR(c.dot(b), 0.0f, 1e-4f);
}
TEST(Vec3, Length) { EXPECT_NEAR(Vec3(3, 4, 0).length(), 5.0f, EPS); }
TEST(Vec3, NormalizedHasUnitLength) { EXPECT_NEAR(Vec3(3, -4, 12).normalized().length(), 1.0f, EPS); }
TEST(Vec3, NormalizedKeepsDirection) { expectVec(Vec3(0, 0, 5).normalized(), Vec3(0, 0, 1)); }
TEST(Vec3, NormalizedZeroVectorIsZero) { expectVec(Vec3().normalized(), Vec3(0, 0, 0)); }

TEST(MathUtils, RadiansFromDegrees)
{
	EXPECT_NEAR(math::radians(180.0f), math::PI, EPS);
	EXPECT_NEAR(math::radians(90.0f), math::PI / 2.0f, EPS);
}
TEST(MathUtils, DegreesFromRadians) { EXPECT_NEAR(math::degrees(math::PI), 180.0f, EPS); }
TEST(MathUtils, DegreesRadiansRoundTrip) { EXPECT_NEAR(math::degrees(math::radians(37.0f)), 37.0f, 1e-4f); }
TEST(MathUtils, ClampInsideRangeUnchanged) { EXPECT_FLOAT_EQ(math::clamp(0.5f, 0.0f, 1.0f), 0.5f); }
TEST(MathUtils, ClampBelowReturnsLow) { EXPECT_FLOAT_EQ(math::clamp(-3.0f, 0.0f, 1.0f), 0.0f); }
TEST(MathUtils, ClampAboveReturnsHigh) { EXPECT_FLOAT_EQ(math::clamp(7.0f, 0.0f, 1.0f), 1.0f); }
TEST(MathUtils, LerpEndpointsAndMiddle)
{
	EXPECT_NEAR(math::lerp(2.0f, 10.0f, 0.0f), 2.0f, EPS);
	EXPECT_NEAR(math::lerp(2.0f, 10.0f, 1.0f), 10.0f, EPS);
	EXPECT_NEAR(math::lerp(2.0f, 10.0f, 0.5f), 6.0f, EPS);
}
TEST(MathUtils, SmoothstepEndpointsAndMiddle)
{
	EXPECT_NEAR(math::smoothstep(0.0f), 0.0f, EPS);
	EXPECT_NEAR(math::smoothstep(1.0f), 1.0f, EPS);
	EXPECT_NEAR(math::smoothstep(0.5f), 0.5f, EPS);
}
TEST(MathUtils, SmoothstepClampsOutsideRange)
{
	EXPECT_NEAR(math::smoothstep(-1.0f), 0.0f, EPS);
	EXPECT_NEAR(math::smoothstep(2.0f), 1.0f, EPS);
}
TEST(MathUtils, SmoothstepIsFlatterThanLinearNearEnds) { EXPECT_LT(math::smoothstep(0.1f), 0.1f); }
