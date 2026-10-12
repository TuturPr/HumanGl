#include <gtest/gtest.h>

#include "../support/DrawHelpers.hpp"
#include "../support/PoseChannels.hpp"
#include "animation/AnimationLibrary.hpp"
#include "skeleton/JointLimits.hpp"

#include <algorithm>
#include <cmath>

using namespace testing_support;

namespace {

constexpr float STEP = 1.0f / 120.0f;
constexpr float ANGLE_TOLERANCE = 2.0f;
constexpr float MAX_STEP_CHANGE = 6.0f;

struct ClipInfo
{
	const char* name;
	float period;	// seconds for one full cycle
};

const ClipInfo CLIPS[] = {{"Idle", 7.0f}, {"Walk", 1.1f}, {"Jump", 1.9f}};

// About three cycles, and never shorter than 5 seconds.
int sampleCount(const ClipInfo& clip)
{
	return static_cast<int>(std::max(5.0f, 3.0f * clip.period) / STEP);
}

const IAnimation& clipNamed(const AnimationLibrary& library, const char* name)
{
	const IAnimation* animation = library.find(name);
	EXPECT_NE(animation, nullptr) << name;
	static const AnimationLibrary empty;
	return animation ? *animation : empty.at(0);
}

Pose sampleAt(const char* name, float t)
{
	static const AnimationLibrary library = AnimationLibrary::createDefault();
	return clipNamed(library, name).sample(t);
}

}

TEST(Clips, LibraryContainsIdleWalkAndJump)
{
	const AnimationLibrary library = AnimationLibrary::createDefault();
	EXPECT_EQ(library.size(), 3u);
	for (const ClipInfo& clip : CLIPS)
		EXPECT_NE(library.find(clip.name), nullptr) << clip.name;
}

TEST(Clips, LibraryFindReturnsNullForUnknownName)
{
	EXPECT_EQ(AnimationLibrary::createDefault().find("Moonwalk"), nullptr);
}

TEST(Clips, NamesMatchTheirLibraryEntries)
{
	const AnimationLibrary library = AnimationLibrary::createDefault();
	for (const ClipInfo& clip : CLIPS)
		EXPECT_EQ(library.find(clip.name)->name(), clip.name);
}

TEST(Clips, RawPosesStayWithinJointLimitsWithSplineTolerance)
{
	const JointLimits limits;
	for (const ClipInfo& clip : CLIPS)
	{
		float worst = 0.0f;
		float worstTime = 0.0f;
		std::size_t worstChannel = 0;
		for (int i = 0; i < sampleCount(clip); ++i)
		{
			const Pose raw = sampleAt(clip.name, static_cast<float>(i) * STEP);
			const std::vector<float> a = channels(raw);
			const std::vector<float> b = channels(limits.clamp(raw));
			for (std::size_t c = 0; c < a.size(); ++c)
				if (std::abs(a[c] - b[c]) > worst)
				{
					worst = std::abs(a[c] - b[c]);
					worstTime = static_cast<float>(i) * STEP;
					worstChannel = c;
				}
		}
		EXPECT_LE(worst, ANGLE_TOLERANCE)
			<< clip.name << ": " << channelName(worstChannel) << " exceeds its limit by " << worst
			<< " degrees at t=" << worstTime;
	}
}

TEST(Clips, RawPosesNeverBendKneesOrElbowsBackwardsByMoreThanTolerance)
{
	for (const ClipInfo& clip : CLIPS)
		for (int i = 0; i < sampleCount(clip); ++i)
		{
			const Pose p = sampleAt(clip.name, static_cast<float>(i) * STEP);
			const float t = static_cast<float>(i) * STEP;
			EXPECT_GE(p.leftLeg.kneeFlex, -ANGLE_TOLERANCE) << clip.name << " t=" << t;
			EXPECT_GE(p.rightLeg.kneeFlex, -ANGLE_TOLERANCE) << clip.name << " t=" << t;
			EXPECT_GE(p.leftArm.elbowFlex, -ANGLE_TOLERANCE) << clip.name << " t=" << t;
			EXPECT_GE(p.rightArm.elbowFlex, -ANGLE_TOLERANCE) << clip.name << " t=" << t;
		}
}

TEST(Clips, ClampedPosesAreExactlyWithinLimits)
{
	const JointLimits limits;
	for (const ClipInfo& clip : CLIPS)
		for (int i = 0; i < sampleCount(clip); ++i)
			ASSERT_TRUE(limits.contains(limits.clamp(sampleAt(clip.name, static_cast<float>(i) * STEP))))
				<< clip.name;
}

TEST(Clips, MotionIsSmoothBetweenConsecutiveSamples)
{
	for (const ClipInfo& clip : CLIPS)
	{
		float worst = 0.0f;
		float worstTime = 0.0f;
		std::size_t worstChannel = 0;
		std::vector<float> previous = channels(sampleAt(clip.name, 0.0f));
		for (int i = 1; i < sampleCount(clip); ++i)
		{
			const std::vector<float> current = channels(sampleAt(clip.name, static_cast<float>(i) * STEP));
			for (std::size_t c = 0; c < current.size(); ++c)
				if (std::abs(current[c] - previous[c]) > worst)
				{
					worst = std::abs(current[c] - previous[c]);
					worstTime = static_cast<float>(i) * STEP;
					worstChannel = c;
				}
			previous = current;
		}
		EXPECT_LE(worst, MAX_STEP_CHANGE)
			<< clip.name << ": " << channelName(worstChannel) << " changes by " << worst
			<< " in one 1/120 s step at t=" << worstTime;
	}
}

TEST(Clips, PosesArePeriodic)
{
	for (const ClipInfo& clip : CLIPS)
		for (float t : {0.0f, 0.13f, 0.5f, 0.77f, 1.0f})
		{
			const std::vector<float> a = channels(sampleAt(clip.name, t));
			const std::vector<float> b = channels(sampleAt(clip.name, t + clip.period));
			for (std::size_t c = 0; c < a.size(); ++c)
				EXPECT_NEAR(a[c], b[c], 0.02f) << clip.name << " " << channelName(c) << " t=" << t;
		}
}

TEST(Clips, NoJumpAcrossTheLoopSeam)
{
	for (const ClipInfo& clip : CLIPS)
	{
		const std::vector<float> before = channels(sampleAt(clip.name, clip.period - STEP / 4.0f));
		const std::vector<float> after = channels(sampleAt(clip.name, clip.period + STEP / 4.0f));
		for (std::size_t c = 0; c < before.size(); ++c)
			EXPECT_NEAR(before[c], after[c], 1.5f) << clip.name << " " << channelName(c);
	}
}

TEST(Clips, JumpStartsOnTheGround)
{
	EXPECT_FLOAT_EQ(sampleAt("Jump", 0.0f).rootOffset.y, 0.0f);
}

TEST(Clips, JumpLeavesTheGroundByMoreThanThirtyCentimetres)
{
	float highest = 0.0f;
	for (int i = 0; i < sampleCount(CLIPS[2]); ++i)
		highest = std::max(highest, sampleAt("Jump", static_cast<float>(i) * STEP).rootOffset.y);
	EXPECT_GT(highest, 0.3f);
}

TEST(Clips, JumpHeightIsNeverNegative)
{
	for (int i = 0; i < sampleCount(CLIPS[2]); ++i)
		EXPECT_GE(sampleAt("Jump", static_cast<float>(i) * STEP).rootOffset.y, 0.0f);
}

TEST(Clips, JumpStaysInPlaceHorizontally)
{
	for (int i = 0; i < sampleCount(CLIPS[2]); ++i)
	{
		const Pose p = sampleAt("Jump", static_cast<float>(i) * STEP);
		EXPECT_FLOAT_EQ(p.rootOffset.x, 0.0f);
		EXPECT_FLOAT_EQ(p.rootOffset.z, 0.0f);
	}
}

TEST(Clips, WalkAndIdleDoNotLiftTheRoot)
{
	for (const char* name : {"Walk", "Idle"})
		for (int i = 0; i < 300; ++i)
			EXPECT_FLOAT_EQ(sampleAt(name, static_cast<float>(i) * STEP).rootOffset.y, 0.0f) << name;
}

TEST(Clips, WalkLegsAreHalfACycleApart)
{
	const float period = CLIPS[1].period;
	for (float t : {0.0f, 0.1f, 0.35f, 0.6f, 0.9f})
	{
		const Pose a = sampleAt("Walk", t);
		const Pose b = sampleAt("Walk", t + period / 2.0f);
		EXPECT_NEAR(a.leftLeg.hipFlex, b.rightLeg.hipFlex, 0.02f) << t;
		EXPECT_NEAR(a.leftLeg.kneeFlex, b.rightLeg.kneeFlex, 0.02f) << t;
		EXPECT_NEAR(a.rightLeg.hipFlex, b.leftLeg.hipFlex, 0.02f) << t;
	}
}

TEST(Clips, WalkLegHipsAreNegativelyCorrelated)
{
	double meanL = 0.0, meanR = 0.0;
	const int n = sampleCount(CLIPS[1]);
	for (int i = 0; i < n; ++i)
	{
		const Pose p = sampleAt("Walk", static_cast<float>(i) * STEP);
		meanL += p.leftLeg.hipFlex;
		meanR += p.rightLeg.hipFlex;
	}
	meanL /= n;
	meanR /= n;
	double covariance = 0.0;
	for (int i = 0; i < n; ++i)
	{
		const Pose p = sampleAt("Walk", static_cast<float>(i) * STEP);
		covariance += (p.leftLeg.hipFlex - meanL) * (p.rightLeg.hipFlex - meanR);
	}
	EXPECT_LT(covariance, 0.0);
}

TEST(Clips, WalkSwingsEachArmOppositeToTheLegOnItsSide)
{
	const int n = sampleCount(CLIPS[1]);
	for (bool left : {true, false})
	{
		double meanLeg = 0.0, meanArm = 0.0;
		for (int i = 0; i < n; ++i)
		{
			const Pose p = sampleAt("Walk", static_cast<float>(i) * STEP);
			meanLeg += left ? p.leftLeg.hipFlex : p.rightLeg.hipFlex;
			meanArm += left ? p.leftArm.shoulderFlex : p.rightArm.shoulderFlex;
		}
		meanLeg /= n;
		meanArm /= n;
		double covariance = 0.0;
		for (int i = 0; i < n; ++i)
		{
			const Pose p = sampleAt("Walk", static_cast<float>(i) * STEP);
			covariance += ((left ? p.leftLeg.hipFlex : p.rightLeg.hipFlex) - meanLeg)
						* ((left ? p.leftArm.shoulderFlex : p.rightArm.shoulderFlex) - meanArm);
		}
		EXPECT_LT(covariance, 0.0) << (left ? "left" : "right") << " arm swings with its own leg";
	}
}

TEST(Clips, WalkKneesOnlyFoldBackwardAndFeetAreNeverBothOffTheGroundAtOnce)
{
	// Both legs having a large knee flex at the same time would mean no leg supports the body.
	for (int i = 0; i < 300; ++i)
	{
		const Pose p = sampleAt("Walk", static_cast<float>(i) * STEP);
		EXPECT_LT(std::min(p.leftLeg.kneeFlex, p.rightLeg.kneeFlex), 25.0f);
	}
}

TEST(Clips, WalkMovesTheLegsAMeaningfulAmount)
{
	float lo = 1e9f, hi = -1e9f;
	for (int i = 0; i < 200; ++i)
	{
		const float hip = sampleAt("Walk", static_cast<float>(i) * STEP).leftLeg.hipFlex;
		lo = std::min(lo, hip);
		hi = std::max(hi, hip);
	}
	EXPECT_GT(hi - lo, 30.0f);
}

TEST(Clips, IdleMovesOnlySlightly)
{
	for (int i = 0; i < sampleCount(CLIPS[0]); i += 10)
	{
		const Pose p = sampleAt("Idle", static_cast<float>(i) * STEP);
		EXPECT_LT(std::abs(p.rootRoll), 5.0f);
		EXPECT_LT(p.leftLeg.kneeFlex, 15.0f);
	}
}

TEST(Clips, LowestPointIsOnTheFloorExceptDuringJumpFlight)
{
	const JointLimits limits;
	const Character character;
	for (const ClipInfo& clip : CLIPS)
		for (int i = 0; i < sampleCount(clip); ++i)
		{
			const float t = static_cast<float>(i) * STEP;
			const Pose pose = limits.clamp(sampleAt(clip.name, t));
			const float lowest = lowestDrawnY(character, pose);
			if (pose.rootOffset.y <= 0.0f)
				EXPECT_NEAR(lowest, 0.0f, 1e-3f) << clip.name << " t=" << t;
			else
				EXPECT_NEAR(lowest, pose.rootOffset.y, 1e-3f) << clip.name << " t=" << t;
		}
}

TEST(Clips, JumpFlightIsContinuousInHeight)
{
	float previous = sampleAt("Jump", 0.0f).rootOffset.y;
	for (int i = 1; i < sampleCount(CLIPS[2]); ++i)
	{
		const float y = sampleAt("Jump", static_cast<float>(i) * STEP).rootOffset.y;
		EXPECT_LT(std::abs(y - previous), 0.03f) << i;
		previous = y;
	}
}

TEST(Clips, EveryClipDrawsAllPartsOnceThroughTheCharacter)
{
	const Character character;
	for (const ClipInfo& clip : CLIPS)
	{
		MatrixStack stack;
		RecordingDrawer drawer;
		character.draw(stack, JointLimits{}.clamp(sampleAt(clip.name, 0.4f)), drawer);
		EXPECT_EQ(drawer.calls.size(), 15u) << clip.name;
	}
}
