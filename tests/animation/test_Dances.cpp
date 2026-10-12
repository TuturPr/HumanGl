#include <gtest/gtest.h>

#include "../support/DrawHelpers.hpp"
#include "../support/PoseChannels.hpp"
#include "animation/AnimationLibrary.hpp"
#include "geometry/RoundedBox.hpp"
#include "skeleton/JointLimits.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

using namespace testing_support;

namespace {

constexpr float STEP = 1.0f / 120.0f;
constexpr float DISCO_PERIOD = 2.0f;
constexpr float KUNG_FU_PERIOD = 3.6f;
constexpr int CYCLES = 3;
constexpr float MAX_STEP_CHANGE = 6.0f;
constexpr float FOOT_LIFT_LIMIT = 0.12f;	// a foot may rise at most this much off the floor

const IAnimation& clip(const char* name)
{
	static const AnimationLibrary library = AnimationLibrary::createDefault();
	const IAnimation* found = library.find(name);
	if (!found)
		throw std::invalid_argument(std::string("unknown clip: ") + name);
	return *found;
}

struct Frame
{
	std::array<float, BODY_PART_COUNT> lowest;
	std::array<Vec3, BODY_PART_COUNT> centre;
	float lowestY;
};

// The body as the Animator would show it at time t (pose clamped to the joint limits).
Frame frameAt(const char* name, float t)
{
	static const MeshData shape = makeRoundedBox(0.2f, 6);
	static const Character character(shape.positions);
	static const JointLimits limits;
	Frame f;
	f.lowest.fill(1e9f);
	f.centre.fill(Vec3{});
	f.lowestY = 1e9f;
	MatrixStack stack;
	RecordingDrawer drawer;
	character.draw(stack, limits.clamp(clip(name).sample(t)), drawer);
	for (const RecordingDrawer::Call& call : drawer.calls)
	{
		const std::size_t k = index(call.part);
		Vec3 sum;
		for (const Vec3& p : shape.positions)
		{
			const Vec3 w = call.matrix.transformPoint(p);
			sum += w;
			f.lowest[k] = std::min(f.lowest[k], w.y);
		}
		f.centre[k] = sum / static_cast<float>(shape.positions.size());
		f.lowestY = std::min(f.lowestY, f.lowest[k]);
	}
	return f;
}

float lowestOf(const Frame& f, BodyPart part) { return f.lowest[index(part)]; }
float heightOf(const Frame& f, BodyPart part) { return f.centre[index(part)].y; }

int samplesFor(float period) { return static_cast<int>(CYCLES * period / STEP); }

// Largest amount (degrees) by which any raw channel leaves its range, over several cycles.
float worstLimitExcess(const char* name, float period, float* atTime, std::size_t* atChannel)
{
	const JointLimits limits;
	float worst = 0.0f;
	for (int i = 0; i < samplesFor(period); ++i)
	{
		const float t = static_cast<float>(i) * STEP;
		const Pose raw = clip(name).sample(t);
		if (limits.contains(raw))
			continue;
		const std::vector<float> a = channels(raw);
		const std::vector<float> b = channels(limits.clamp(raw));
		for (std::size_t c = 0; c < a.size(); ++c)
			if (std::abs(a[c] - b[c]) > worst)
			{
				worst = std::abs(a[c] - b[c]);
				*atTime = t;
				*atChannel = c;
			}
	}
	return worst;
}

void expectLoopsAfterOneCycle(const char* name, float period)
{
	for (float t = 0.0f; t < period; t += 0.037f)
	{
		const std::vector<float> a = channels(clip(name).sample(t));
		const std::vector<float> b = channels(clip(name).sample(t + period));
		const std::vector<float> c2 = channels(clip(name).sample(t + 2.0f * period));
		for (std::size_t c = 0; c < a.size(); ++c)
		{
			EXPECT_NEAR(wrap180(b[c] - a[c]), 0.0f, 0.02f) << name << " " << channelName(c) << " t=" << t;
			EXPECT_NEAR(wrap180(c2[c] - a[c]), 0.0f, 0.05f) << name << " " << channelName(c) << " t=" << t;
		}
	}
}

// Includes the step across the loop seam, since the samples run over several cycles.
void expectSmallSteps(const char* name, float period)
{
	std::vector<float> previous = channels(clip(name).sample(0.0f));
	for (int i = 1; i < samplesFor(period); ++i)
	{
		const std::vector<float> current = channels(clip(name).sample(static_cast<float>(i) * STEP));
		for (std::size_t c = 0; c < current.size(); ++c)
			EXPECT_LE(std::abs(wrap180(current[c] - previous[c])), MAX_STEP_CHANGE)
				<< name << ": " << channelName(c) << " at t=" << static_cast<float>(i) * STEP;
		previous = current;
	}
}

}

// ------------------------------------------------------------------ Disco ---

TEST(Disco, RawPoseIsStrictlyInsideJointLimits)
{
	float time = 0.0f;
	std::size_t channel = 0;
	const float excess = worstLimitExcess("Disco", DISCO_PERIOD, &time, &channel);
	EXPECT_EQ(excess, 0.0f) << channelName(channel) << " exceeds its limit by " << excess << " degrees at t=" << time;
}

TEST(Disco, LoopsExactlyAfterOneCycle)
{
	expectLoopsAfterOneCycle("Disco", DISCO_PERIOD);
}

TEST(Disco, MotionIsSmooth)
{
	expectSmallSteps("Disco", DISCO_PERIOD);
}

TEST(Disco, IsDancedInPlace)
{
	for (int i = 0; i < samplesFor(DISCO_PERIOD); ++i)
	{
		const Pose p = clip("Disco").sample(static_cast<float>(i) * STEP);
		EXPECT_FLOAT_EQ(p.rootOffset.x, 0.0f);
		EXPECT_FLOAT_EQ(p.rootOffset.y, 0.0f);
		EXPECT_FLOAT_EQ(p.rootOffset.z, 0.0f);
	}
}

TEST(Disco, BothFeetStayOnTheFloorAndNothingSinksBelowIt)
{
	for (int i = 0; i < samplesFor(DISCO_PERIOD); i += 2)
	{
		const float t = static_cast<float>(i) * STEP;
		const Frame f = frameAt("Disco", t);
		EXPECT_GE(f.lowestY, -1e-3f) << "body part below the floor at t=" << t;
		EXPECT_NEAR(f.lowestY, 0.0f, 1e-3f) << "t=" << t;
		for (BodyPart foot : {BodyPart::FootLeft, BodyPart::FootRight})
			EXPECT_LT(lowestOf(f, foot), FOOT_LIFT_LIMIT) << bodyPartName(foot) << " t=" << t;
		// Weight is always carried by at least one foot.
		EXPECT_LT(std::min(lowestOf(f, BodyPart::FootLeft), lowestOf(f, BodyPart::FootRight)), 0.01f) << "t=" << t;
	}
}

TEST(Disco, RightHandPointsAboveTheHeadOnTheUpBeats)
{
	// The finger points up on beats 1 and 2 of each half-cycle phrase (phase 0 and 0.25: t = 0 and 0.5 s).
	for (float t : {0.0f, 0.5f, DISCO_PERIOD, DISCO_PERIOD + 0.5f})
	{
		const Frame f = frameAt("Disco", t);
		EXPECT_GT(heightOf(f, BodyPart::HandRight), heightOf(f, BodyPart::Head) + 0.2f) << "t=" << t;
	}
}

TEST(Disco, RightHandComesDownToHipLevelOnTheDownBeats)
{
	for (float t : {0.25f, 0.75f})
	{
		const Frame f = frameAt("Disco", t);
		EXPECT_LT(heightOf(f, BodyPart::HandRight), heightOf(f, BodyPart::Torso)) << "t=" << t;
	}
}

TEST(Disco, HipsSwayToBothSidesEveryCycle)
{
	float left = 0.0f, right = 0.0f;
	int crossings = 0;
	float previous = clip("Disco").sample(0.0f).rootRoll;
	for (int i = 1; i <= static_cast<int>(DISCO_PERIOD / STEP); ++i)
	{
		const float roll = clip("Disco").sample(static_cast<float>(i) * STEP).rootRoll;
		left = std::max(left, roll);
		right = std::min(right, roll);
		crossings += (roll > 0.0f) != (previous > 0.0f);
		previous = roll;
	}
	EXPECT_GT(left, 4.0f);
	EXPECT_LT(right, -4.0f);
	EXPECT_GE(crossings, 4);	// sways left and right at least twice per cycle
}

// ---------------------------------------------------------------- Kung-fu ---

TEST(KungFu, RawPoseIsStrictlyInsideJointLimits)
{
	float time = 0.0f;
	std::size_t channel = 0;
	const float excess = worstLimitExcess("Kung-fu", KUNG_FU_PERIOD, &time, &channel);
	EXPECT_EQ(excess, 0.0f) << channelName(channel) << " exceeds its limit by " << excess << " degrees at t=" << time;
}

TEST(KungFu, LoopsExactlyAfterOneCycle)
{
	expectLoopsAfterOneCycle("Kung-fu", KUNG_FU_PERIOD);
}

TEST(KungFu, MotionIsSmooth)
{
	expectSmallSteps("Kung-fu", KUNG_FU_PERIOD);
}

TEST(KungFu, StaysInPlaceWithTheLowestPointOnTheFloor)
{
	for (int i = 0; i < samplesFor(KUNG_FU_PERIOD); i += 4)
	{
		const float t = static_cast<float>(i) * STEP;
		const Pose p = clip("Kung-fu").sample(t);
		EXPECT_FLOAT_EQ(p.rootOffset.x, 0.0f);
		EXPECT_FLOAT_EQ(p.rootOffset.z, 0.0f);
		const Frame f = frameAt("Kung-fu", t);
		EXPECT_NEAR(f.lowestY, 0.0f, 1e-3f) << "t=" << t;
	}
}

TEST(KungFu, RightFootKicksUpToHipHeightWhileTheLeftFootStaysPlanted)
{
	// The snap kick is at 0.64 of the 3.6 s cycle.
	const float kickTime = 0.64f * KUNG_FU_PERIOD;
	float highest = 0.0f;
	for (float t = 0.55f * KUNG_FU_PERIOD; t <= 0.75f * KUNG_FU_PERIOD; t += STEP)
	{
		const Frame f = frameAt("Kung-fu", t);
		highest = std::max(highest, heightOf(f, BodyPart::FootRight));
		EXPECT_LT(lowestOf(f, BodyPart::FootLeft), 0.02f) << "supporting foot lifts at t=" << t;
	}
	const Frame kick = frameAt("Kung-fu", kickTime);
	EXPECT_NEAR(heightOf(kick, BodyPart::FootRight), heightOf(kick, BodyPart::Torso), 0.3f)
		<< "kicking foot not at hip/waist height";
	EXPECT_GT(heightOf(kick, BodyPart::FootRight), 0.8f);
	EXPECT_GT(highest, 0.8f);
	EXPECT_LT(highest, heightOf(kick, BodyPart::Head)) << "the kick should not go over the head";
}

TEST(KungFu, BothFeetAreFlatOnTheFloorInTheHorseStance)
{
	for (float t : {0.0f, 0.12f * KUNG_FU_PERIOD, 0.24f * KUNG_FU_PERIOD, 0.36f * KUNG_FU_PERIOD})
	{
		const Frame f = frameAt("Kung-fu", t);
		EXPECT_NEAR(lowestOf(f, BodyPart::FootLeft), 0.0f, 0.02f) << "t=" << t;
		EXPECT_NEAR(lowestOf(f, BodyPart::FootRight), 0.0f, 0.02f) << "t=" << t;
	}
}

TEST(KungFu, TorsoTwistsBothWaysIntoThePunches)
{
	// Right punch at 0.12, left punch at 0.24 of the cycle.
	const Pose right = clip("Kung-fu").sample(0.12f * KUNG_FU_PERIOD);
	const Pose left = clip("Kung-fu").sample(0.24f * KUNG_FU_PERIOD);
	EXPECT_GT(right.rootYaw, 12.0f);
	EXPECT_LT(left.rootYaw, -12.0f);
	// The punching arm is the extended one.
	EXPECT_LT(right.rightArm.elbowFlex, 10.0f);
	EXPECT_GT(right.leftArm.elbowFlex, 90.0f);
	EXPECT_LT(left.leftArm.elbowFlex, 10.0f);
	EXPECT_GT(left.rightArm.elbowFlex, 90.0f);
}

TEST(KungFu, HeadKeepsFacingForwardWhileTheTorsoTwists)
{
	float widestTwist = 0.0f;
	for (int i = 0; i < samplesFor(KUNG_FU_PERIOD); ++i)
	{
		const float t = static_cast<float>(i) * STEP;
		const Pose p = clip("Kung-fu").sample(t);
		EXPECT_NEAR(wrap180(p.neckYaw + p.rootYaw), 0.0f, 1.0f) << "t=" << t;
		widestTwist = std::max(widestTwist, std::abs(p.rootYaw));
	}
	EXPECT_GT(widestTwist, 12.0f);
}
