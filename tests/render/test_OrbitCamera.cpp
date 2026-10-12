#include <gtest/gtest.h>

#include "math/MathUtils.hpp"
#include "render/OrbitCamera.hpp"

#include <cmath>

namespace {
constexpr float EPS = 1e-3f;

void expectVec(const Vec3& a, const Vec3& b)
{
	EXPECT_NEAR(a.x, b.x, EPS);
	EXPECT_NEAR(a.y, b.y, EPS);
	EXPECT_NEAR(a.z, b.z, EPS);
}

OrbitCamera facingFront()
{
	OrbitCamera c({0.0f, 1.0f, 0.0f}, 5.0f);
	c.orbit(-c.yaw(), -c.pitch());
	return c;
}
}

TEST(OrbitCamera, ConstructorStoresTargetAndDistance)
{
	const OrbitCamera c({1, 2, 3}, 6.0f);
	expectVec(c.target(), {1, 2, 3});
	EXPECT_FLOAT_EQ(c.distance(), 6.0f);
}

TEST(OrbitCamera, ConstructorClampsDistance)
{
	EXPECT_FLOAT_EQ(OrbitCamera({0, 0, 0}, 0.1f).distance(), OrbitCamera::MIN_DISTANCE);
	EXPECT_FLOAT_EQ(OrbitCamera({0, 0, 0}, 500.0f).distance(), OrbitCamera::MAX_DISTANCE);
}

TEST(OrbitCamera, PositionIsAtTheDistanceFromTheTarget)
{
	OrbitCamera c({0, 1, 0}, 4.5f);
	c.orbit(70.0f, 30.0f);
	EXPECT_NEAR((c.position() - c.target()).length(), 4.5f, EPS);
}

TEST(OrbitCamera, ZeroYawAndPitchPutsTheCameraInFrontOnPlusZ)
{
	expectVec(facingFront().position(), {0.0f, 1.0f, 5.0f});
}

TEST(OrbitCamera, PositiveYawMovesTheCameraTowardPlusX)
{
	OrbitCamera c = facingFront();
	c.orbit(90.0f, 0.0f);
	expectVec(c.position(), {5.0f, 1.0f, 0.0f});
}

TEST(OrbitCamera, PositivePitchRaisesTheCamera)
{
	OrbitCamera c = facingFront();
	c.orbit(0.0f, 30.0f);
	EXPECT_NEAR(c.position().y, 1.0f + 5.0f * std::sin(math::radians(30.0f)), EPS);
}

TEST(OrbitCamera, PitchIsClampedToItsRange)
{
	OrbitCamera c;
	c.orbit(0.0f, 500.0f);
	EXPECT_FLOAT_EQ(c.pitch(), OrbitCamera::MAX_PITCH);
	c.orbit(0.0f, -1000.0f);
	EXPECT_FLOAT_EQ(c.pitch(), OrbitCamera::MIN_PITCH);
}

TEST(OrbitCamera, YawWrapsIntoHalfTurn)
{
	OrbitCamera c = facingFront();
	c.orbit(400.0f, 0.0f);
	EXPECT_NEAR(c.yaw(), 40.0f, EPS);
	c.orbit(-100.0f, 0.0f);
	EXPECT_NEAR(c.yaw(), -60.0f, EPS);
}

TEST(OrbitCamera, FullTurnComesBackToTheSamePosition)
{
	OrbitCamera c;
	const Vec3 before = c.position();
	c.orbit(360.0f, 0.0f);
	expectVec(c.position(), before);
}

TEST(OrbitCamera, ZoomMultipliesTheDistance)
{
	OrbitCamera c({0, 0, 0}, 4.0f);
	c.zoom(2.0f);
	EXPECT_FLOAT_EQ(c.distance(), 8.0f);
	c.zoom(0.5f);
	EXPECT_FLOAT_EQ(c.distance(), 4.0f);
}

TEST(OrbitCamera, ZoomIsClampedToItsRange)
{
	OrbitCamera c({0, 0, 0}, 4.0f);
	c.zoom(1000.0f);
	EXPECT_FLOAT_EQ(c.distance(), OrbitCamera::MAX_DISTANCE);
	c.zoom(1e-6f);
	EXPECT_FLOAT_EQ(c.distance(), OrbitCamera::MIN_DISTANCE);
}

TEST(OrbitCamera, NonPositiveZoomFactorIsIgnored)
{
	OrbitCamera c({0, 0, 0}, 4.0f);
	c.zoom(0.0f);
	c.zoom(-2.0f);
	EXPECT_FLOAT_EQ(c.distance(), 4.0f);
}

TEST(OrbitCamera, ViewMatrixPutsTheCameraAtTheOrigin)
{
	OrbitCamera c;
	c.orbit(33.0f, 20.0f);
	expectVec(c.viewMatrix().transformPoint(c.position()), {0, 0, 0});
}

TEST(OrbitCamera, ViewMatrixPutsTheTargetStraightAhead)
{
	OrbitCamera c;
	c.orbit(-50.0f, 15.0f);
	expectVec(c.viewMatrix().transformPoint(c.target()), {0.0f, 0.0f, -c.distance()});
}

TEST(OrbitCamera, WorldUpStaysUpOnScreen)
{
	OrbitCamera c;
	c.orbit(40.0f, 10.0f);
	EXPECT_GT(c.viewMatrix().transformDirection({0, 1, 0}).y, 0.0f);
}

TEST(OrbitCamera, ProjectionKeepsTheTargetInsideTheClipVolume)
{
	const OrbitCamera c;
	const Mat4 mvp = c.projectionMatrix(16.0f / 9.0f) * c.viewMatrix();
	const Vec3 t = c.target();
	const Vec3 clip = mvp.transformPoint(t);	// x, y, z before the perspective divide
	const float w = mvp(3, 0) * t.x + mvp(3, 1) * t.y + mvp(3, 2) * t.z + mvp(3, 3);
	EXPECT_GT(w, 0.0f);
	EXPECT_NEAR(clip.x / w, 0.0f, EPS);
	EXPECT_NEAR(clip.y / w, 0.0f, EPS);
	EXPECT_GT(clip.z / w, -1.0f);
	EXPECT_LT(clip.z / w, 1.0f);
}
