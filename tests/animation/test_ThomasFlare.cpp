#include <gtest/gtest.h>

#include "../support/DrawHelpers.hpp"
#include "../support/PoseChannels.hpp"
#include "animation/AnimationLibrary.hpp"
#include "geometry/RoundedBox.hpp"
#include "skeleton/JointLimits.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

using namespace testing_support;

namespace {

constexpr float STEP = 1.0f / 120.0f;
constexpr float PERIOD = 1.5f;
constexpr int SAMPLES = static_cast<int>(2.0f * PERIOD / STEP);	// two revolutions
constexpr int SAMPLES_PER_REVOLUTION = static_cast<int>(PERIOD / STEP + 0.5f);
constexpr float TO_DEGREES = 180.0f / 3.14159265358979f;

const Pose& rawPose(int i)
{
	static const std::vector<Pose> poses = [] {
		const AnimationLibrary library = AnimationLibrary::createDefault();
		const IAnimation* flare = library.find("Thomas flare");
		EXPECT_NE(flare, nullptr);
		std::vector<Pose> v;
		for (int k = 0; k <= SAMPLES; ++k)
			v.push_back(flare ? flare->sample(static_cast<float>(k) * STEP) : Pose{});
		return v;
	}();
	return poses[static_cast<std::size_t>(i)];
}

struct Frame
{
	float t;
	Pose pose;	// clamped, as the Animator feeds the Character
	std::array<float, BODY_PART_COUNT> lowest;
	std::array<Vec3, BODY_PART_COUNT> centre;
	std::array<bool, BODY_PART_COUNT> drawn;
	Mat4 torso;
	BodyPart lowestPart;
	float lowestY;
};

const std::vector<Frame>& frames()
{
	static const std::vector<Frame> all = [] {
		const MeshData shape = makeRoundedBox(0.2f, 6);
		const Character character(shape.positions);
		const JointLimits limits;
		std::vector<Frame> v;
		for (int i = 0; i <= SAMPLES; ++i)
		{
			Frame f;
			f.t = static_cast<float>(i) * STEP;
			f.pose = limits.clamp(rawPose(i));
			f.lowest.fill(1e9f);
			f.centre.fill(Vec3{});
			f.drawn.fill(false);
			MatrixStack stack;
			RecordingDrawer drawer;
			character.draw(stack, f.pose, drawer);
			f.lowestPart = BodyPart::Torso;
			f.lowestY = 1e9f;
			for (const RecordingDrawer::Call& call : drawer.calls)
			{
				const std::size_t k = index(call.part);
				f.drawn[k] = true;
				Vec3 sum;
				for (const Vec3& p : shape.positions)
				{
					const Vec3 w = call.matrix.transformPoint(p);
					sum += w;
					f.lowest[k] = std::min(f.lowest[k], w.y);
				}
				f.centre[k] = sum / static_cast<float>(shape.positions.size());
				if (f.lowest[k] < f.lowestY)
				{
					f.lowestY = f.lowest[k];
					f.lowestPart = call.part;
				}
				if (call.part == BodyPart::Torso)
					f.torso = call.matrix;
			}
			v.push_back(f);
		}
		return v;
	}();
	return all;
}

bool isHand(BodyPart p)
{
	return p == BodyPart::HandLeft || p == BodyPart::HandRight;
}

float wrap180(float degrees)
{
	return degrees - 360.0f * std::floor((degrees + 180.0f) / 360.0f);
}

}

TEST(ThomasFlare, RawPoseIsAlwaysWithinJointLimits)
{
	const JointLimits limits;
	float worst = 0.0f, worstTime = 0.0f;
	std::size_t worstChannel = 0;
	for (int i = 0; i <= SAMPLES; ++i)
	{
		const std::vector<float> a = channels(rawPose(i));
		const std::vector<float> b = channels(limits.clamp(rawPose(i)));
		for (std::size_t c = 0; c < a.size(); ++c)
			if (std::abs(a[c] - b[c]) > worst)
			{
				worst = std::abs(a[c] - b[c]);
				worstTime = static_cast<float>(i) * STEP;
				worstChannel = c;
			}
	}
	EXPECT_LE(worst, 1e-3f) << channelName(worstChannel) << " exceeds its limit by " << worst
						   << " degrees at t=" << worstTime;
}

TEST(ThomasFlare, LowestPointOfTheBodyIsAHandOnTheFloor)
{
	for (const Frame& f : frames())
	{
		EXPECT_TRUE(isHand(f.lowestPart)) << "lowest part is " << bodyPartName(f.lowestPart) << " at t=" << f.t;
		EXPECT_NEAR(f.lowestY, 0.0f, 1e-3f) << "t=" << f.t;
	}
}

TEST(ThomasFlare, NothingButTheHandsComesCloseToTheFloor)
{
	const BodyPart others[] = {BodyPart::Torso, BodyPart::Head, BodyPart::Neck,
		BodyPart::ThighLeft, BodyPart::ThighRight, BodyPart::LowerLegLeft, BodyPart::LowerLegRight,
		BodyPart::FootLeft, BodyPart::FootRight};
	for (const Frame& f : frames())
		for (BodyPart part : others)
			EXPECT_GE(f.lowest[index(part)], 0.15f)
				<< bodyPartName(part) << " is " << f.lowest[index(part)] << " above the floor at t=" << f.t;
}

TEST(ThomasFlare, BothHandsTakeTurnsCarryingTheBody)
{
	int left = 0, right = 0;
	for (int i = 0; i < SAMPLES_PER_REVOLUTION; ++i)
	{
		const BodyPart part = frames()[static_cast<std::size_t>(i)].lowestPart;
		left += part == BodyPart::HandLeft;
		right += part == BodyPart::HandRight;
	}
	EXPECT_GT(left, SAMPLES_PER_REVOLUTION * 25 / 100) << "left hand lowest " << left << "/" << SAMPLES_PER_REVOLUTION;
	EXPECT_GT(right, SAMPLES_PER_REVOLUTION * 25 / 100) << "right hand lowest " << right << "/" << SAMPLES_PER_REVOLUTION;
}

TEST(ThomasFlare, APlantedHandBarelySlides)
{
	for (std::size_t i = 1; i < frames().size(); ++i)
		for (BodyPart hand : {BodyPart::HandLeft, BodyPart::HandRight})
		{
			const Frame& a = frames()[i - 1];
			const Frame& b = frames()[i];
			const std::size_t k = index(hand);
			if (a.lowest[k] > 0.004f || b.lowest[k] > 0.004f)
				continue;
			const float dx = b.centre[k].x - a.centre[k].x;
			const float dz = b.centre[k].z - a.centre[k].z;
			EXPECT_LT(std::sqrt(dx * dx + dz * dz), 0.05f)
				<< bodyPartName(hand) << " slides by " << std::sqrt(dx * dx + dz * dz) << " at t=" << b.t;
		}
}

TEST(ThomasFlare, RootYawTurnsMinusThreeHundredSixtyDegreesPerCycle)
{
	for (int i = 0; i <= SAMPLES; ++i)
		EXPECT_NEAR(rawPose(i).rootYaw, -360.0f * static_cast<float>(i) * STEP / PERIOD, 0.05f) << i;
	EXPECT_NEAR(rawPose(SAMPLES_PER_REVOLUTION).rootYaw, -360.0f, 1.5f);
}

TEST(ThomasFlare, TorsoHeadingCompletesOneFullTurnPerRevolution)
{
	// Direction the torso's long axis points to (toward the head), seen from above.
	auto heading = [](const Frame& f) {
		const Vec3 axis = f.torso.transformDirection({0, 1, 0});
		return std::atan2(axis.x, axis.z) * TO_DEGREES;
	};
	float total = 0.0f;
	float previous = heading(frames()[0]);
	std::vector<float> unwrapped = {0.0f};
	for (std::size_t i = 1; i < frames().size(); ++i)
	{
		const float h = heading(frames()[i]);
		total += wrap180(h - previous);
		previous = h;
		unwrapped.push_back(total);
	}
	EXPECT_NEAR(unwrapped[static_cast<std::size_t>(SAMPLES_PER_REVOLUTION)], -360.0f, 10.0f);
	EXPECT_NEAR(unwrapped.back(), -720.0f, 15.0f);
	for (std::size_t i = 1; i < unwrapped.size(); ++i)
		EXPECT_LT(unwrapped[i] - unwrapped[i - 1], 0.5f) << "heading turns backwards at t=" << frames()[i].t;
}

TEST(ThomasFlare, BodyStaysRoughlyHorizontal)
{
	for (const Frame& f : frames())
	{
		const Vec3 axis = f.torso.transformDirection({0, 1, 0}).normalized();
		EXPECT_LT(std::abs(axis.y), 0.35f) << "torso axis y=" << axis.y << " at t=" << f.t;
	}
}

TEST(ThomasFlare, MotionIsSmoothBetweenConsecutiveSamples)
{
	for (std::size_t i = 1; i < frames().size(); ++i)
	{
		const std::vector<float> a = channels(frames()[i - 1].pose);
		const std::vector<float> b = channels(frames()[i].pose);
		for (std::size_t c = 0; c < a.size(); ++c)
		{
			if (c == 3)	// rootYaw
				continue;
			EXPECT_LE(std::abs(b[c] - a[c]), 6.0f)
				<< channelName(c) << " changes by " << std::abs(b[c] - a[c]) << " at t=" << frames()[i].t;
		}
	}
}

TEST(ThomasFlare, LoopsAfterOneRevolution)
{
	for (int i = 0; i + SAMPLES_PER_REVOLUTION <= SAMPLES; i += 3)
	{
		const std::vector<float> a = channels(rawPose(i));
		const std::vector<float> b = channels(rawPose(i + SAMPLES_PER_REVOLUTION));
		for (std::size_t c = 0; c < a.size(); ++c)
		{
			const float diff = c == 3 ? wrap180(b[c] - a[c]) : b[c] - a[c];
			EXPECT_NEAR(diff, 0.0f, c == 3 ? 0.5f : 0.05f)
				<< channelName(c) << " at t=" << static_cast<float>(i) * STEP;
		}
	}
}
