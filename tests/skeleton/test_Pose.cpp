#include <gtest/gtest.h>

#include "skeleton/Pose.hpp"

namespace {
constexpr float EPS = 1e-5f;

Pose samplePose(float base)
{
	Pose p;
	p.rootOffset = {base, base + 1.0f, base + 2.0f};
	p.rootYaw = base * 2.0f;
	p.rootPitch = base * 3.0f;
	p.rootRoll = base * 4.0f;
	p.neckPitch = base * 5.0f;
	p.neckYaw = base * 6.0f;
	p.neckRoll = base * 7.0f;
	p.leftArm = {base, base + 1, base + 2, base + 3};
	p.rightArm = {base + 4, base + 5, base + 6, base + 7};
	p.leftLeg = {base + 8, base + 9, base + 10, base + 11};
	p.rightLeg = {base + 12, base + 13, base + 14, base + 15};
	return p;
}
}

TEST(Pose, DefaultPoseIsAllZero)
{
	const Pose p;
	EXPECT_EQ(p.rootOffset.y, 0.0f);
	EXPECT_EQ(p.rootYaw, 0.0f);
	EXPECT_EQ(p.leftArm.elbowFlex, 0.0f);
	EXPECT_EQ(p.rightLeg.kneeFlex, 0.0f);
}

TEST(Pose, ArmPoseAddition)
{
	const ArmPose r = ArmPose{1, 2, 3, 4} + ArmPose{10, 20, 30, 40};
	EXPECT_NEAR(r.shoulderFlex, 11.0f, EPS);
	EXPECT_NEAR(r.shoulderAbduct, 22.0f, EPS);
	EXPECT_NEAR(r.elbowFlex, 33.0f, EPS);
	EXPECT_NEAR(r.wristFlex, 44.0f, EPS);
}

TEST(Pose, ArmPoseScaling)
{
	const ArmPose r = ArmPose{1, 2, 3, 4} * 2.0f;
	EXPECT_NEAR(r.shoulderFlex, 2.0f, EPS);
	EXPECT_NEAR(r.shoulderAbduct, 4.0f, EPS);
	EXPECT_NEAR(r.elbowFlex, 6.0f, EPS);
	EXPECT_NEAR(r.wristFlex, 8.0f, EPS);
}

TEST(Pose, LegPoseAddition)
{
	const LegPose r = LegPose{1, 2, 3, 4} + LegPose{-1, -2, -3, -4};
	EXPECT_NEAR(r.hipFlex, 0.0f, EPS);
	EXPECT_NEAR(r.hipAbduct, 0.0f, EPS);
	EXPECT_NEAR(r.kneeFlex, 0.0f, EPS);
	EXPECT_NEAR(r.ankleFlex, 0.0f, EPS);
}

TEST(Pose, LegPoseScaling)
{
	const LegPose r = LegPose{1, 2, 3, 4} * -0.5f;
	EXPECT_NEAR(r.hipFlex, -0.5f, EPS);
	EXPECT_NEAR(r.hipAbduct, -1.0f, EPS);
	EXPECT_NEAR(r.kneeFlex, -1.5f, EPS);
	EXPECT_NEAR(r.ankleFlex, -2.0f, EPS);
}

TEST(Pose, PoseAdditionAddsEveryChannel)
{
	const Pose a = samplePose(1.0f);
	const Pose b = samplePose(2.0f);
	const Pose r = a + b;
	EXPECT_NEAR(r.rootOffset.x, 3.0f, EPS);
	EXPECT_NEAR(r.rootOffset.y, 5.0f, EPS);
	EXPECT_NEAR(r.rootOffset.z, 7.0f, EPS);
	EXPECT_NEAR(r.rootYaw, 6.0f, EPS);
	EXPECT_NEAR(r.rootPitch, 9.0f, EPS);
	EXPECT_NEAR(r.rootRoll, 12.0f, EPS);
	EXPECT_NEAR(r.neckPitch, 15.0f, EPS);
	EXPECT_NEAR(r.neckYaw, 18.0f, EPS);
	EXPECT_NEAR(r.neckRoll, 21.0f, EPS);
	EXPECT_NEAR(r.leftArm.wristFlex, a.leftArm.wristFlex + b.leftArm.wristFlex, EPS);
	EXPECT_NEAR(r.rightArm.shoulderFlex, a.rightArm.shoulderFlex + b.rightArm.shoulderFlex, EPS);
	EXPECT_NEAR(r.leftLeg.kneeFlex, a.leftLeg.kneeFlex + b.leftLeg.kneeFlex, EPS);
	EXPECT_NEAR(r.rightLeg.ankleFlex, a.rightLeg.ankleFlex + b.rightLeg.ankleFlex, EPS);
}

TEST(Pose, PoseScalingScalesEveryChannel)
{
	const Pose a = samplePose(1.0f);
	const Pose r = a * 3.0f;
	EXPECT_NEAR(r.rootOffset.z, a.rootOffset.z * 3.0f, EPS);
	EXPECT_NEAR(r.rootRoll, a.rootRoll * 3.0f, EPS);
	EXPECT_NEAR(r.neckRoll, a.neckRoll * 3.0f, EPS);
	EXPECT_NEAR(r.leftArm.elbowFlex, a.leftArm.elbowFlex * 3.0f, EPS);
	EXPECT_NEAR(r.rightLeg.hipAbduct, a.rightLeg.hipAbduct * 3.0f, EPS);
}

TEST(Pose, LerpAtZeroGivesFirstPose)
{
	const Pose a = samplePose(1.0f);
	const Pose r = lerp(a, samplePose(9.0f), 0.0f);
	EXPECT_NEAR(r.rootYaw, a.rootYaw, EPS);
	EXPECT_NEAR(r.leftLeg.kneeFlex, a.leftLeg.kneeFlex, EPS);
}

TEST(Pose, LerpAtOneGivesSecondPose)
{
	const Pose b = samplePose(9.0f);
	const Pose r = lerp(samplePose(1.0f), b, 1.0f);
	EXPECT_NEAR(r.rootYaw, b.rootYaw, EPS);
	EXPECT_NEAR(r.rightArm.elbowFlex, b.rightArm.elbowFlex, EPS);
	EXPECT_NEAR(r.rootOffset.y, b.rootOffset.y, EPS);
}

TEST(Pose, LerpAtHalfGivesMidpoint)
{
	const Pose a = samplePose(0.0f);
	const Pose b = samplePose(10.0f);
	const Pose r = lerp(a, b, 0.5f);
	EXPECT_NEAR(r.rootYaw, 10.0f, EPS);
	EXPECT_NEAR(r.leftLeg.kneeFlex, (a.leftLeg.kneeFlex + b.leftLeg.kneeFlex) / 2.0f, EPS);
	EXPECT_NEAR(r.rootOffset.x, 5.0f, EPS);
}
