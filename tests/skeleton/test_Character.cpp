#include <gtest/gtest.h>

#include "../support/DrawHelpers.hpp"

#include <stdexcept>

using namespace testing_support;

namespace {
constexpr float EPS = 1e-3f;

Pose deepSquat()
{
	Pose p;
	p.leftLeg = {110.0f, 5.0f, 130.0f, 20.0f};
	p.rightLeg = p.leftLeg;
	return p;
}
}

TEST(Character, DefaultPoseLowestPointIsOnTheFloor)
{
	EXPECT_NEAR(lowestDrawnY(Character{}, Pose{}), 0.0f, EPS);
}

TEST(Character, RootOffsetYLiftsTheBodyAboveTheFloor)
{
	Pose p;
	p.rootOffset.y = 0.5f;
	EXPECT_NEAR(lowestDrawnY(Character{}, p), 0.5f, EPS);
}

TEST(Character, RootOffsetXZMovesBodyOnTheFloorWithoutLifting)
{
	Pose p;
	p.rootOffset = {2.0f, 0.0f, -1.0f};
	EXPECT_NEAR(lowestDrawnY(Character{}, p), 0.0f, EPS);
}

TEST(Character, DrawsEachPartOnce)
{
	MatrixStack stack;
	RecordingDrawer drawer;
	Character{}.draw(stack, Pose{}, drawer);
	EXPECT_EQ(drawer.calls.size(), 15u);
}

TEST(Character, DrawRestoresMatrixStackDepth)
{
	MatrixStack stack;
	RecordingDrawer drawer;
	Character{}.draw(stack, deepSquat(), drawer);
	EXPECT_EQ(stack.depth(), 1u);
}

TEST(Character, ContactLowestIsNegativeHeightOfModelAtOrigin)
{
	// The model at origin hangs below the shoulder line; lowestPoint is that (negative) height.
	const float lowest = Character{}.contact(Pose{}).lowest;
	EXPECT_LT(lowest, -1.0f);
	EXPECT_GT(lowest, -2.0f);
}

TEST(Character, ContactLowestIgnoresRootOffsetY)
{
	Pose lifted;
	lifted.rootOffset.y = 3.0f;
	EXPECT_NEAR(Character{}.contact(lifted).lowest, Character{}.contact(Pose{}).lowest, EPS);
}

TEST(Character, LongerLegsKeepFeetOnTheFloor)
{
	Character c;
	c.model().dimensions().setSize(BodyPart::ThighLeft, {0.15f, 0.9f, 0.16f});
	c.model().dimensions().setSize(BodyPart::ThighRight, {0.15f, 0.9f, 0.16f});
	c.model().dimensions().setSize(BodyPart::LowerLegLeft, {0.11f, 0.8f, 0.11f});
	c.model().dimensions().setSize(BodyPart::LowerLegRight, {0.11f, 0.8f, 0.11f});
	EXPECT_NEAR(lowestDrawnY(c, Pose{}), 0.0f, EPS);
}

TEST(Character, LongerLegsRaiseTheTorso)
{
	Character normal;
	Character tall;
	tall.model().dimensions().setSize(BodyPart::ThighLeft, {0.15f, 0.9f, 0.16f});
	tall.model().dimensions().setSize(BodyPart::ThighRight, {0.15f, 0.9f, 0.16f});
	MatrixStack s1, s2;
	RecordingDrawer a, b;
	normal.draw(s1, Pose{}, a);
	tall.draw(s2, Pose{}, b);
	EXPECT_GT(b.center(BodyPart::Torso).y, a.center(BodyPart::Torso).y + 0.4f);
}

TEST(Character, ShorterLegsKeepFeetOnTheFloor)
{
	Character c;
	c.model().dimensions().setSize(BodyPart::LowerLegLeft, {0.11f, 0.1f, 0.11f});
	c.model().dimensions().setSize(BodyPart::LowerLegRight, {0.11f, 0.1f, 0.11f});
	EXPECT_NEAR(lowestDrawnY(c, Pose{}), 0.0f, EPS);
}

TEST(Character, DeepKneeBendKeepsFeetOnTheFloor)
{
	EXPECT_NEAR(lowestDrawnY(Character{}, deepSquat()), 0.0f, EPS);
}

TEST(Character, DeepKneeBendLowersTheBody)
{
	Character c;
	MatrixStack s1, s2;
	RecordingDrawer standing, squatting;
	c.draw(s1, Pose{}, standing);
	c.draw(s2, deepSquat(), squatting);
	EXPECT_LT(squatting.center(BodyPart::Head).y, standing.center(BodyPart::Head).y - 0.3f);
}

TEST(Character, TorsoPitchKeepsBodyOnTheFloor)
{
	Pose p;
	p.rootPitch = 40.0f;
	EXPECT_NEAR(lowestDrawnY(Character{}, p), 0.0f, EPS);
}

TEST(Character, LeftLegRaisedStillLeavesTheOtherFootOnTheFloor)
{
	Pose p;
	p.leftLeg = {80.0f, 0.0f, 90.0f, 0.0f};
	EXPECT_NEAR(lowestDrawnY(Character{}, p), 0.0f, EPS);
}

TEST(Character, CustomContactHullIsUsedToFindTheFloor)
{
	// A single contact point at the unit-shape centre: the floor touches the
	// lowest part centre rather than its corner, so the body sits lower.
	Character centres(std::vector<Vec3>{{0.0f, 0.0f, 0.0f}});
	const float low = lowestDrawnY(centres, Pose{});
	EXPECT_LT(low, 0.0f);
	EXPECT_GT(low, -0.1f);
}

TEST(Character, ModelAccessorsReferToTheSameModel)
{
	Character c;
	c.model().setBonusPartsEnabled(false);
	const Character& cc = c;
	EXPECT_FALSE(cc.model().bonusPartsEnabled());
}

// ---------------------------------------------------------------- contact ---

TEST(CharacterContact, StandingSupportIsBetweenTheFeetNearTheFloorLevel)
{
	const Character::Contact c = Character{}.contact(Pose{});
	EXPECT_NEAR(c.support.x, 0.0f, EPS);
	EXPECT_NEAR(c.support.y, c.lowest, Character::CONTACT_TOLERANCE);
}

TEST(CharacterContact, SupportYIsCloseToTheLowestPoint)
{
	const Character::Contact c = Character{}.contact(deepSquat());
	EXPECT_GE(c.support.y, c.lowest - EPS);
	EXPECT_LE(c.support.y, c.lowest + Character::CONTACT_TOLERANCE);
}

TEST(CharacterContact, LiftingLeftFootMovesSupportUnderTheRightFoot)
{
	Pose p;
	p.leftLeg = {0.0f, 0.0f, 90.0f, 0.0f};
	const Character::Contact c = Character{}.contact(p);
	const RecordingDrawer d = drawModel(Character{}.model(), Pose{});
	EXPECT_NEAR(c.support.x, d.center(BodyPart::FootRight).x, 0.06f);
	EXPECT_LT(c.support.x, 0.0f);
}

TEST(CharacterContact, LiftingRightFootMovesSupportUnderTheLeftFoot)
{
	Pose p;
	p.rightLeg = {0.0f, 0.0f, 90.0f, 0.0f};
	EXPECT_GT(Character{}.contact(p).support.x, 0.03f);
}

TEST(CharacterContact, SupportIgnoresRootOffset)
{
	Pose p;
	p.rootOffset = {3.0f, 2.0f, -4.0f};
	const Character::Contact a = Character{}.contact(p);
	const Character::Contact b = Character{}.contact(Pose{});
	expectVecNear(a.support, b.support);
}

TEST(CharacterContact, SupportMovesContinuouslyWhenTheLiftedFootChanges)
{
	Pose p;
	float previous = 0.0f;
	for (int k = 0; k <= 90; ++k)
	{
		p.leftLeg.kneeFlex = static_cast<float>(k);
		const float x = Character{}.contact(p).support.x;
		if (k > 0)
		{
			EXPECT_LT(std::abs(x - previous), 0.05f) << k;
		}
		previous = x;
	}
}

// ------------------------------------------------------------ supportPinning ---

namespace {

// Weighted average x/z of the drawn hull points that touch the floor.
Vec3 drawnSupport(const Character& character, const Pose& pose)
{
	MatrixStack stack;
	RecordingDrawer drawer;
	character.draw(stack, pose, drawer);
	std::vector<Vec3> points;
	float lowest = 1e9f;
	for (const auto& call : drawer.calls)
		for (const Vec3& corner : cubeCorners())
		{
			points.push_back(call.matrix.transformPoint(corner));
			lowest = std::min(lowest, points.back().y);
		}
	Vec3 sum;
	float total = 0.0f;
	for (const Vec3& p : points)
	{
		const float w = 1.0f - (p.y - lowest) / Character::CONTACT_TOLERANCE;
		if (w > 0.0f)
		{
			sum += p * w;
			total += w;
		}
	}
	return sum / total;
}

std::vector<Pose> variedPoses()
{
	std::vector<Pose> poses(4);
	poses[0].rootYaw = 90.0f;
	poses[1].rootYaw = 37.0f;
	poses[1].leftLeg = {0.0f, 0.0f, 80.0f, 0.0f};
	poses[2].rootPitch = 80.0f;
	poses[2].rootRoll = 20.0f;
	poses[2].leftArm = {170.0f, 0.0f, 0.0f, 0.0f};
	poses[3].rightLeg = {60.0f, 0.0f, 100.0f, 0.0f};
	poses[3].rootYaw = -120.0f;
	return poses;
}

}

TEST(CharacterPinning, FullPinningKeepsContactPointUnderRootOffset)
{
	for (Pose p : variedPoses())
	{
		p.supportPinning = 1.0f;
		p.rootOffset = {1.5f, 0.0f, -2.0f};
		const Vec3 s = drawnSupport(Character{}, p);
		EXPECT_NEAR(s.x, 1.5f, 1e-3f);
		EXPECT_NEAR(s.z, -2.0f, 1e-3f);
	}
}

TEST(CharacterPinning, FullPinningWithZeroOffsetKeepsContactAtTheOrigin)
{
	for (Pose p : variedPoses())
	{
		p.supportPinning = 1.0f;
		const Vec3 s = drawnSupport(Character{}, p);
		EXPECT_NEAR(s.x, 0.0f, 1e-3f);
		EXPECT_NEAR(s.z, 0.0f, 1e-3f);
	}
}

TEST(CharacterPinning, ZeroPinningDrawsTheModelOriginAtRootOffsetXZ)
{
	Pose p;
	p.rootYaw = 30.0f;
	p.leftLeg = {0.0f, 0.0f, 80.0f, 0.0f};
	p.rootOffset = {1.0f, 0.0f, 2.0f};
	MatrixStack stack;
	RecordingDrawer drawer;
	Character{}.draw(stack, p, drawer);
	// The torso top (shoulder line middle) is exactly above the root offset.
	const Vec3 shoulderMiddle = drawer.matrix(BodyPart::Torso).transformPoint({0.0f, 0.5f, 0.0f});
	EXPECT_NEAR(shoulderMiddle.x, 1.0f, 1e-3f);
	EXPECT_NEAR(shoulderMiddle.z, 2.0f, 1e-3f);
}

TEST(CharacterPinning, HalfPinningIsHalfwayBetweenNoneAndFull)
{
	Pose p;
	p.leftLeg = {0.0f, 0.0f, 90.0f, 0.0f};
	const Vec3 support = Character{}.contact(p).support;
	MatrixStack s0, sh;
	RecordingDrawer d0, dh;
	Character{}.draw(s0, p, d0);
	p.supportPinning = 0.5f;
	Character{}.draw(sh, p, dh);
	const Vec3 delta = dh.center(BodyPart::Torso) - d0.center(BodyPart::Torso);
	EXPECT_NEAR(delta.x, -support.x * 0.5f, 1e-3f);
	EXPECT_NEAR(delta.z, -support.z * 0.5f, 1e-3f);
	EXPECT_NEAR(delta.y, 0.0f, 1e-3f);
}

TEST(CharacterPinning, PinningDoesNotChangeHeight)
{
	for (Pose p : variedPoses())
	{
		p.supportPinning = 1.0f;
		EXPECT_NEAR(lowestDrawnY(Character{}, p), 0.0f, EPS);
	}
}

TEST(CharacterPinning, PoseAdditionSumsSupportPinning)
{
	Pose a, b;
	a.supportPinning = 0.25f;
	b.supportPinning = 0.5f;
	EXPECT_NEAR((a + b).supportPinning, 0.75f, 1e-6f);
}

TEST(CharacterPinning, PoseScalingScalesSupportPinning)
{
	Pose a;
	a.supportPinning = 0.8f;
	EXPECT_NEAR((a * 0.5f).supportPinning, 0.4f, 1e-6f);
}

TEST(CharacterPinning, PoseLerpBlendsSupportPinning)
{
	Pose a, b;
	b.supportPinning = 1.0f;
	EXPECT_NEAR(lerp(a, b, 0.3f).supportPinning, 0.3f, 1e-6f);
	EXPECT_FLOAT_EQ(Pose{}.supportPinning, 0.0f);
}

TEST(Character, EmptyContactHullThrows)
{
	EXPECT_THROW(Character(std::vector<Vec3>{}), std::invalid_argument);
}
