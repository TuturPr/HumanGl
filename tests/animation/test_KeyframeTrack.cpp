#include <gtest/gtest.h>

#include "animation/KeyframeTrack.hpp"
#include "skeleton/Pose.hpp"

#include <cmath>
#include <stdexcept>

namespace {

using FloatTrack = KeyframeTrack<float>;

FloatTrack makeTrack()
{
	return FloatTrack({{0.0f, 0.0f}, {0.2f, 1.0f}, {0.5f, 0.2f}, {0.8f, 0.9f}});
}

}

TEST(KeyframeTrack, PassesThroughEveryKeyExactly)
{
	const FloatTrack t = makeTrack();
	EXPECT_NEAR(t.sample(0.0f), 0.0f, 1e-5f);
	EXPECT_NEAR(t.sample(0.2f), 1.0f, 1e-5f);
	EXPECT_NEAR(t.sample(0.5f), 0.2f, 1e-5f);
	EXPECT_NEAR(t.sample(0.8f), 0.9f, 1e-5f);
}

TEST(KeyframeTrack, FirstKeyNeedNotBeAtPhaseZero)
{
	const FloatTrack t({{0.25f, 3.0f}, {0.75f, -3.0f}});
	EXPECT_NEAR(t.sample(0.25f), 3.0f, 1e-5f);
	EXPECT_NEAR(t.sample(0.75f), -3.0f, 1e-5f);
}

TEST(KeyframeTrack, SampleAtPhasePlusOneEqualsSampleAtPhase)
{
	const FloatTrack t = makeTrack();
	for (float p : {0.0f, 0.1f, 0.33f, 0.5f, 0.77f, 0.99f})
		EXPECT_NEAR(t.sample(p + 1.0f), t.sample(p), 1e-4f) << p;
}

TEST(KeyframeTrack, SampleAtPhaseMinusOneEqualsSampleAtPhase)
{
	const FloatTrack t = makeTrack();
	for (float p : {0.0f, 0.1f, 0.33f, 0.5f, 0.77f, 0.99f})
		EXPECT_NEAR(t.sample(p - 1.0f), t.sample(p), 1e-4f) << p;
}

TEST(KeyframeTrack, FarAwayPhasesStillWrap)
{
	const FloatTrack t = makeTrack();
	EXPECT_NEAR(t.sample(0.3f + 7.0f), t.sample(0.3f), 1e-3f);
	EXPECT_NEAR(t.sample(0.3f - 5.0f), t.sample(0.3f), 1e-3f);
}

TEST(KeyframeTrack, ConstructorRejectsFewerThanTwoKeys)
{
	EXPECT_THROW(FloatTrack({}), std::invalid_argument);
	EXPECT_THROW(FloatTrack({{0.0f, 1.0f}}), std::invalid_argument);
}

TEST(KeyframeTrack, ConstructorRejectsPhaseOfOneOrMore)
{
	EXPECT_THROW(FloatTrack({{0.0f, 1.0f}, {1.0f, 2.0f}}), std::invalid_argument);
	EXPECT_THROW(FloatTrack({{0.0f, 1.0f}, {1.5f, 2.0f}}), std::invalid_argument);
}

TEST(KeyframeTrack, ConstructorRejectsNegativePhase)
{
	EXPECT_THROW(FloatTrack({{-0.1f, 1.0f}, {0.5f, 2.0f}}), std::invalid_argument);
}

TEST(KeyframeTrack, ConstructorRejectsDecreasingPhases)
{
	EXPECT_THROW(FloatTrack({{0.5f, 1.0f}, {0.2f, 2.0f}}), std::invalid_argument);
}

TEST(KeyframeTrack, ConstructorRejectsDuplicatePhases)
{
	EXPECT_THROW(FloatTrack({{0.2f, 1.0f}, {0.2f, 2.0f}}), std::invalid_argument);
}

TEST(KeyframeTrack, ConstantKeysGiveConstantCurve)
{
	const FloatTrack t({{0.0f, 4.0f}, {0.3f, 4.0f}, {0.6f, 4.0f}});
	for (float p = 0.0f; p < 1.0f; p += 0.037f)
		EXPECT_NEAR(t.sample(p), 4.0f, 1e-4f);
}

TEST(KeyframeTrack, CurveHasNoJumpAnywhere)
{
	const FloatTrack t = makeTrack();
	const float step = 1e-3f;
	for (float p = -0.5f; p < 1.5f; p += step)
		EXPECT_LT(std::abs(t.sample(p + step) - t.sample(p)), 0.02f) << p;
}

TEST(KeyframeTrack, NoJumpAcrossTheLoopBoundary)
{
	const FloatTrack t = makeTrack();
	const float eps = 1e-4f;
	EXPECT_NEAR(t.sample(1.0f - eps), t.sample(0.0f + eps), 0.01f);
	EXPECT_NEAR(t.sample(-eps), t.sample(eps), 0.01f);
}

TEST(KeyframeTrack, NoJumpAcrossLoopBoundaryWhenFirstKeyIsNotAtZero)
{
	const FloatTrack t({{0.3f, 0.0f}, {0.6f, 1.0f}, {0.9f, 0.5f}});
	for (float boundary : {0.0f, 0.3f, 0.6f, 0.9f, 1.0f})
		EXPECT_NEAR(t.sample(boundary - 1e-4f), t.sample(boundary + 1e-4f), 0.01f) << boundary;
}

TEST(KeyframeTrack, VelocityIsContinuousAtEveryKey)
{
	const FloatTrack t = makeTrack();
	const float h = 2e-4f;
	for (float key : {0.2f, 0.5f, 0.8f})
	{
		const float left = (t.sample(key) - t.sample(key - h)) / h;
		const float right = (t.sample(key + h) - t.sample(key)) / h;
		EXPECT_NEAR(left, right, 0.1f) << key;
	}
}

TEST(KeyframeTrack, VelocityIsContinuousAtTheLoopSeam)
{
	const FloatTrack t = makeTrack();
	const float h = 2e-4f;
	const float left = (t.sample(1.0f) - t.sample(1.0f - h)) / h;
	const float right = (t.sample(1.0f + h) - t.sample(1.0f)) / h;
	EXPECT_NEAR(left, right, 0.1f);
}

TEST(KeyframeTrack, VelocityIsContinuousAtAKeyAwayFromZero)
{
	const FloatTrack t({{0.3f, 0.0f}, {0.6f, 1.0f}, {0.9f, 0.5f}});
	const float h = 2e-4f;
	for (float key : {0.3f, 0.6f, 0.9f})
	{
		const float left = (t.sample(key) - t.sample(key - h)) / h;
		const float right = (t.sample(key + h) - t.sample(key)) / h;
		EXPECT_NEAR(left, right, 0.1f) << key;
	}
}

TEST(KeyframeTrack, SlopeIsNotZeroForcedAtKeys)
{
	// A Catmull-Rom curve through a monotone ramp keeps moving through its middle key.
	const FloatTrack t({{0.0f, 0.0f}, {0.25f, 1.0f}, {0.5f, 2.0f}, {0.75f, 3.0f}});
	const float h = 1e-3f;
	EXPECT_GT((t.sample(0.5f + h) - t.sample(0.5f - h)) / (2.0f * h), 1.0f);
}

TEST(KeyframeTrack, WorksWithLegPose)
{
	const KeyframeTrack<LegPose> t({
		{0.0f, {10.0f, 0.0f, 5.0f, 1.0f}},
		{0.5f, {-10.0f, 4.0f, 50.0f, -1.0f}},
	});
	const LegPose a = t.sample(0.0f);
	const LegPose b = t.sample(0.5f);
	EXPECT_NEAR(a.hipFlex, 10.0f, 1e-4f);
	EXPECT_NEAR(a.kneeFlex, 5.0f, 1e-4f);
	EXPECT_NEAR(b.hipFlex, -10.0f, 1e-4f);
	EXPECT_NEAR(b.hipAbduct, 4.0f, 1e-4f);
	EXPECT_NEAR(b.kneeFlex, 50.0f, 1e-4f);
	EXPECT_NEAR(b.ankleFlex, -1.0f, 1e-4f);
}

TEST(KeyframeTrack, LegPoseLoopsLikeFloat)
{
	const KeyframeTrack<LegPose> t({
		{0.0f, {10.0f, 0.0f, 5.0f, 1.0f}},
		{0.4f, {-10.0f, 4.0f, 50.0f, -1.0f}},
		{0.7f, {0.0f, 2.0f, 20.0f, 3.0f}},
	});
	const LegPose a = t.sample(0.23f);
	const LegPose b = t.sample(1.23f);
	EXPECT_NEAR(a.hipFlex, b.hipFlex, 1e-3f);
	EXPECT_NEAR(a.kneeFlex, b.kneeFlex, 1e-3f);
}

TEST(KeyframeTrack, EachChannelOfAPoseTrackIsInterpolatedIndependently)
{
	Pose a;
	a.leftLeg.kneeFlex = 10.0f;
	Pose b;
	b.leftLeg.kneeFlex = 90.0f;
	b.rootYaw = 30.0f;
	const KeyframeTrack<Pose> t({{0.0f, a}, {0.5f, b}});
	EXPECT_NEAR(t.sample(0.5f).rootYaw, 30.0f, 1e-4f);
	EXPECT_NEAR(t.sample(0.5f).leftLeg.kneeFlex, 90.0f, 1e-4f);
	EXPECT_NEAR(t.sample(0.0f).rootYaw, 0.0f, 1e-4f);
	EXPECT_NEAR(t.sample(0.0f).rightLeg.kneeFlex, 0.0f, 1e-4f);
}

TEST(KeyframeTrack, TwoKeyTrackIsSymmetricAroundTheMiddle)
{
	const FloatTrack t({{0.0f, 0.0f}, {0.5f, 1.0f}});
	EXPECT_NEAR(t.sample(0.25f), 0.5f, 1e-4f);
	EXPECT_NEAR(t.sample(0.75f), 0.5f, 1e-4f);
}
