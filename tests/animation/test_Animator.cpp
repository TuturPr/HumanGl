#include <gtest/gtest.h>

#include "../support/FakeAnimation.hpp"
#include "animation/Animator.hpp"

#include <cmath>

using testing_support::FakeAnimation;

namespace {

constexpr float EPS = 1e-3f;

FakeAnimation constantPitch(float pitch)
{
	return FakeAnimation("pitch", [pitch](float) {
		Pose p;
		p.rootPitch = pitch;
		return p;
	});
}

FakeAnimation constantYaw(float yaw)
{
	return FakeAnimation("yaw", [yaw](float) {
		Pose p;
		p.rootYaw = yaw;
		return p;
	});
}

// rootYaw grows at 10 degrees per second.
FakeAnimation ramp()
{
	return FakeAnimation("ramp", [](float t) {
		Pose p;
		p.rootYaw = 10.0f * t;
		return p;
	});
}

Animator instantAnimator()
{
	Animator a;
	a.setTransitionDuration(0.0f);
	return a;
}

}

TEST(Animator, StartsWithoutAnimationAndNeutralPose)
{
	const Animator a;
	EXPECT_EQ(a.current(), nullptr);
	EXPECT_FLOAT_EQ(a.pose().rootYaw, 0.0f);
}

TEST(Animator, UpdateWithoutAnimationDoesNothing)
{
	Animator a;
	a.update(1.0f);
	EXPECT_FLOAT_EQ(a.pose().rootPitch, 0.0f);
}

TEST(Animator, PlaySetsCurrentAnimation)
{
	Animator a;
	const FakeAnimation anim = ramp();
	a.play(anim);
	EXPECT_EQ(a.current(), &anim);
}

TEST(Animator, UpdateAdvancesTheAnimationClock)
{
	Animator a = instantAnimator();
	const FakeAnimation anim = ramp();
	a.play(anim);
	a.update(0.5f);
	EXPECT_NEAR(a.pose().rootYaw, 5.0f, EPS);
	a.update(0.5f);
	EXPECT_NEAR(a.pose().rootYaw, 10.0f, EPS);
}

TEST(Animator, SpeedScalesTheClock)
{
	Animator a = instantAnimator();
	const FakeAnimation anim = ramp();
	a.setSpeed(2.0f);
	a.play(anim);
	a.update(0.5f);
	EXPECT_NEAR(a.pose().rootYaw, 10.0f, EPS);
}

TEST(Animator, ZeroSpeedFreezesTheAnimation)
{
	Animator a = instantAnimator();
	const FakeAnimation anim = ramp();
	a.setSpeed(0.0f);
	a.play(anim);
	a.update(3.0f);
	EXPECT_NEAR(a.pose().rootYaw, 0.0f, EPS);
}

TEST(Animator, NegativeSpeedIsClampedToZero)
{
	Animator a;
	a.setSpeed(-3.0f);
	EXPECT_FLOAT_EQ(a.speed(), 0.0f);
}

TEST(Animator, PauseFreezesTime)
{
	Animator a = instantAnimator();
	const FakeAnimation anim = ramp();
	a.play(anim);
	a.update(1.0f);
	a.setPaused(true);
	a.update(5.0f);
	EXPECT_TRUE(a.paused());
	EXPECT_NEAR(a.pose().rootYaw, 10.0f, EPS);
}

TEST(Animator, ResumingAfterPauseContinuesWhereItStopped)
{
	Animator a = instantAnimator();
	const FakeAnimation anim = ramp();
	a.play(anim);
	a.update(1.0f);
	a.setPaused(true);
	a.update(5.0f);
	a.setPaused(false);
	a.update(1.0f);
	EXPECT_NEAR(a.pose().rootYaw, 20.0f, EPS);
}

TEST(Animator, PauseAlsoFreezesTheCrossFade)
{
	Animator a;
	a.setTransitionDuration(1.0f);
	const FakeAnimation anim = constantPitch(40.0f);
	a.play(anim);
	a.setPaused(true);
	a.update(10.0f);
	EXPECT_NEAR(a.pose().rootPitch, 0.0f, EPS);
}

TEST(Animator, FirstFramesOfCrossFadeAreCloseToThePreviousPose)
{
	Animator a;
	a.setTransitionDuration(1.0f);
	const FakeAnimation first = constantPitch(40.0f);
	const FakeAnimation second = constantPitch(-20.0f);
	a.play(first);
	a.update(1.5f);
	ASSERT_NEAR(a.pose().rootPitch, 40.0f, EPS);
	a.play(second);
	a.update(0.01f);
	EXPECT_NEAR(a.pose().rootPitch, 40.0f, 0.5f);
}

TEST(Animator, CrossFadeReachesTheMiddleHalfwayThrough)
{
	Animator a;
	a.setTransitionDuration(1.0f);
	const FakeAnimation first = constantPitch(40.0f);
	const FakeAnimation second = constantPitch(-20.0f);
	a.play(first);
	a.update(1.5f);
	a.play(second);
	a.update(0.5f);
	EXPECT_NEAR(a.pose().rootPitch, 10.0f, EPS);
}

TEST(Animator, AfterTheTransitionTheNewAnimationIsExact)
{
	Animator a;
	a.setTransitionDuration(0.5f);
	const FakeAnimation first = constantPitch(40.0f);
	const FakeAnimation second = constantPitch(-20.0f);
	a.play(first);
	a.update(1.0f);
	a.play(second);
	a.update(0.6f);
	EXPECT_NEAR(a.pose().rootPitch, -20.0f, EPS);
}

TEST(Animator, CrossFadeIsMonotonicBetweenTwoConstantPoses)
{
	Animator a;
	a.setTransitionDuration(1.0f);
	const FakeAnimation first = constantPitch(40.0f);
	const FakeAnimation second = constantPitch(-20.0f);
	a.play(first);
	a.update(1.5f);
	a.play(second);
	float previous = a.pose().rootPitch;
	for (int i = 0; i < 120; ++i)
	{
		a.update(1.0f / 100.0f);
		EXPECT_LE(a.pose().rootPitch, previous + 1e-4f);
		previous = a.pose().rootPitch;
	}
}

TEST(Animator, InterruptedCrossFadeStartsFromTheCurrentPose)
{
	Animator a;
	a.setTransitionDuration(1.0f);
	const FakeAnimation first = constantPitch(40.0f);
	const FakeAnimation second = constantPitch(-20.0f);
	const FakeAnimation third = constantPitch(0.0f);
	a.play(first);
	a.update(1.5f);
	a.play(second);
	a.update(0.5f);
	const float mid = a.pose().rootPitch;
	a.play(third);
	a.update(0.001f);
	EXPECT_NEAR(a.pose().rootPitch, mid, 0.1f);
}

TEST(Animator, ReplayingTheSameAnimationDoesNothing)
{
	Animator a = instantAnimator();
	const FakeAnimation anim = ramp();
	a.play(anim);
	a.update(1.0f);
	a.play(anim);
	a.update(1.0f);
	EXPECT_NEAR(a.pose().rootYaw, 20.0f, EPS);
}

TEST(Animator, PlayingADifferentAnimationRestartsItsClock)
{
	Animator a = instantAnimator();
	const FakeAnimation first = ramp();
	const FakeAnimation second = ramp();
	a.play(first);
	a.update(2.0f);
	a.play(second);
	a.update(1.0f);
	EXPECT_NEAR(a.pose().rootYaw, 10.0f, EPS);
}

TEST(Animator, OutputKneeIsClampedToLimits)
{
	Animator a = instantAnimator();
	const FakeAnimation anim("bad", [](float) {
		Pose p;
		p.leftLeg.kneeFlex = -40.0f;
		p.rightLeg.kneeFlex = -5.0f;
		return p;
	});
	a.play(anim);
	a.update(0.1f);
	EXPECT_FLOAT_EQ(a.pose().leftLeg.kneeFlex, 0.0f);
	EXPECT_FLOAT_EQ(a.pose().rightLeg.kneeFlex, 0.0f);
}

TEST(Animator, OutputElbowIsClampedToLimits)
{
	Animator a = instantAnimator();
	const FakeAnimation anim("bad", [](float) {
		Pose p;
		p.leftArm.elbowFlex = -25.0f;
		p.rightArm.elbowFlex = 400.0f;
		return p;
	});
	a.play(anim);
	a.update(0.1f);
	EXPECT_FLOAT_EQ(a.pose().leftArm.elbowFlex, 0.0f);
	EXPECT_FLOAT_EQ(a.pose().rightArm.elbowFlex, JointLimits{}.arm.elbowFlex.max);
}

TEST(Animator, OutputIsClampedDuringCrossFadeToo)
{
	Animator a;
	a.setTransitionDuration(1.0f);
	const FakeAnimation anim("bad", [](float) {
		Pose p;
		p.leftLeg.kneeFlex = -90.0f;
		return p;
	});
	a.play(anim);
	for (int i = 0; i < 20; ++i)
	{
		a.update(0.05f);
		EXPECT_TRUE(JointLimits{}.contains(a.pose()));
	}
}

TEST(Animator, CustomLimitsAreUsed)
{
	JointLimits limits;
	limits.leg.kneeFlex = {0.0f, 20.0f};
	Animator a(limits);
	a.setTransitionDuration(0.0f);
	const FakeAnimation anim("deep", [](float) {
		Pose p;
		p.leftLeg.kneeFlex = 100.0f;
		return p;
	});
	a.play(anim);
	a.update(0.1f);
	EXPECT_FLOAT_EQ(a.pose().leftLeg.kneeFlex, 20.0f);
}

TEST(Animator, YawCrossFadeTakesTheShortWay)
{
	Animator a;
	a.setTransitionDuration(1.0f);
	const FakeAnimation from = constantYaw(350.0f);
	const FakeAnimation to = constantYaw(10.0f);
	a.play(from);
	a.update(1.5f);
	a.play(to);
	for (int i = 0; i < 100; ++i)
	{
		a.update(0.01f);
		const float yaw = std::remainder(a.pose().rootYaw, 360.0f);
		EXPECT_LE(std::abs(yaw), 10.0f + 1e-2f) << i;
	}
}

TEST(Animator, YawCrossFadeMidpointIsZeroNot180)
{
	Animator a;
	a.setTransitionDuration(1.0f);
	const FakeAnimation from = constantYaw(350.0f);
	const FakeAnimation to = constantYaw(10.0f);
	a.play(from);
	a.update(1.5f);
	a.play(to);
	a.update(0.5f);
	EXPECT_NEAR(std::remainder(a.pose().rootYaw, 360.0f), 0.0f, EPS);
}

TEST(Animator, YawCrossFadeTakesTheShortWayInTheOtherDirectionToo)
{
	Animator a;
	a.setTransitionDuration(1.0f);
	const FakeAnimation from = constantYaw(10.0f);
	const FakeAnimation to = constantYaw(350.0f);
	a.play(from);
	a.update(1.5f);
	a.play(to);
	for (int i = 0; i < 100; ++i)
	{
		a.update(0.01f);
		EXPECT_LE(std::abs(std::remainder(a.pose().rootYaw, 360.0f)), 10.0f + 1e-2f) << i;
	}
}

TEST(Animator, TransitionDurationCannotBeNegative)
{
	Animator a;
	a.setTransitionDuration(-1.0f);
	EXPECT_FLOAT_EQ(a.transitionDuration(), 0.0f);
}

TEST(Animator, TransitionDurationIsStored)
{
	Animator a;
	a.setTransitionDuration(0.75f);
	EXPECT_FLOAT_EQ(a.transitionDuration(), 0.75f);
}

TEST(Animator, ZeroTransitionSnapsToTheNewAnimation)
{
	Animator a = instantAnimator();
	const FakeAnimation first = constantPitch(40.0f);
	const FakeAnimation second = constantPitch(-20.0f);
	a.play(first);
	a.update(0.1f);
	a.play(second);
	a.update(0.001f);
	EXPECT_NEAR(a.pose().rootPitch, -20.0f, EPS);
}
