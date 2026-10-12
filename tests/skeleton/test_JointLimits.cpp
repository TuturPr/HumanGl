#include <gtest/gtest.h>

#include "skeleton/JointLimits.hpp"

TEST(JointRange, ClampKeepsValueInside)
{
	const JointRange r{-10.0f, 20.0f};
	EXPECT_FLOAT_EQ(r.clamp(5.0f), 5.0f);
}

TEST(JointRange, ClampRaisesValueBelowMin)
{
	const JointRange r{-10.0f, 20.0f};
	EXPECT_FLOAT_EQ(r.clamp(-50.0f), -10.0f);
}

TEST(JointRange, ClampLowersValueAboveMax)
{
	const JointRange r{-10.0f, 20.0f};
	EXPECT_FLOAT_EQ(r.clamp(50.0f), 20.0f);
}

TEST(JointRange, ContainsIncludesBounds)
{
	const JointRange r{-10.0f, 20.0f};
	EXPECT_TRUE(r.contains(-10.0f));
	EXPECT_TRUE(r.contains(20.0f));
	EXPECT_TRUE(r.contains(0.0f));
}

TEST(JointRange, ContainsRejectsOutside)
{
	const JointRange r{-10.0f, 20.0f};
	EXPECT_FALSE(r.contains(-10.01f));
	EXPECT_FALSE(r.contains(20.01f));
}

TEST(JointLimits, DefaultPoseIsWithinLimits)
{
	EXPECT_TRUE(JointLimits{}.contains(Pose{}));
}

TEST(JointLimits, KneesCannotBendBackwards)
{
	Pose p;
	p.leftLeg.kneeFlex = -40.0f;
	p.rightLeg.kneeFlex = -1.0f;
	EXPECT_FALSE(JointLimits{}.contains(p));
	const Pose c = JointLimits{}.clamp(p);
	EXPECT_FLOAT_EQ(c.leftLeg.kneeFlex, 0.0f);
	EXPECT_FLOAT_EQ(c.rightLeg.kneeFlex, 0.0f);
}

TEST(JointLimits, ElbowsCannotBendBackwards)
{
	Pose p;
	p.leftArm.elbowFlex = -30.0f;
	p.rightArm.elbowFlex = -0.5f;
	EXPECT_FALSE(JointLimits{}.contains(p));
	const Pose c = JointLimits{}.clamp(p);
	EXPECT_FLOAT_EQ(c.leftArm.elbowFlex, 0.0f);
	EXPECT_FLOAT_EQ(c.rightArm.elbowFlex, 0.0f);
}

TEST(JointLimits, KneeAndElbowMinimumsAreZero)
{
	const JointLimits l;
	EXPECT_FLOAT_EQ(l.leg.kneeFlex.min, 0.0f);
	EXPECT_FLOAT_EQ(l.arm.elbowFlex.min, 0.0f);
}

TEST(JointLimits, KneeIsClampedToItsMaximum)
{
	Pose p;
	p.leftLeg.kneeFlex = 1000.0f;
	EXPECT_FLOAT_EQ(JointLimits{}.clamp(p).leftLeg.kneeFlex, JointLimits{}.leg.kneeFlex.max);
}

TEST(JointLimits, ClampedPoseIsAlwaysContained)
{
	Pose p;
	p.neckPitch = 500.0f;
	p.neckYaw = -500.0f;
	p.neckRoll = 500.0f;
	p.leftArm = {-999.0f, 999.0f, -999.0f, 999.0f};
	p.rightArm = {999.0f, -999.0f, 999.0f, -999.0f};
	p.leftLeg = {-999.0f, 999.0f, -999.0f, 999.0f};
	p.rightLeg = {999.0f, -999.0f, 999.0f, -999.0f};
	EXPECT_TRUE(JointLimits{}.contains(JointLimits{}.clamp(p)));
}

TEST(JointLimits, ClampLeavesValidPoseUnchanged)
{
	Pose p;
	p.leftArm = {30.0f, 10.0f, 45.0f, 5.0f};
	p.rightLeg = {40.0f, 5.0f, 60.0f, -10.0f};
	p.neckPitch = 10.0f;
	const Pose c = JointLimits{}.clamp(p);
	EXPECT_FLOAT_EQ(c.leftArm.shoulderFlex, 30.0f);
	EXPECT_FLOAT_EQ(c.leftArm.elbowFlex, 45.0f);
	EXPECT_FLOAT_EQ(c.rightLeg.kneeFlex, 60.0f);
	EXPECT_FLOAT_EQ(c.neckPitch, 10.0f);
}

TEST(JointLimits, RootIsUnbounded)
{
	Pose p;
	p.rootYaw = 720.0f;
	p.rootPitch = -300.0f;
	p.rootRoll = 400.0f;
	p.rootOffset = {10.0f, 20.0f, 30.0f};
	const Pose c = JointLimits{}.clamp(p);
	EXPECT_FLOAT_EQ(c.rootYaw, 720.0f);
	EXPECT_FLOAT_EQ(c.rootPitch, -300.0f);
	EXPECT_FLOAT_EQ(c.rootRoll, 400.0f);
	EXPECT_FLOAT_EQ(c.rootOffset.y, 20.0f);
	EXPECT_TRUE(JointLimits{}.contains(p));
}
