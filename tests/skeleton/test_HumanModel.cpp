#include <gtest/gtest.h>

#include "../support/DrawHelpers.hpp"

#include "math/MathUtils.hpp"

#include <set>

using namespace testing_support;

namespace {

constexpr float EPS = 1e-4f;

const std::vector<BodyPart>& allParts()
{
	static const std::vector<BodyPart> parts = [] {
		std::vector<BodyPart> v;
		for (std::size_t i = 0; i < BODY_PART_COUNT; ++i)
			v.push_back(static_cast<BodyPart>(i));
		return v;
	}();
	return parts;
}

std::set<BodyPart> movedParts(const RecordingDrawer& before, const RecordingDrawer& after)
{
	std::set<BodyPart> moved;
	for (BodyPart part : allParts())
		if (!sameMatrix(before.matrix(part), after.matrix(part)))
			moved.insert(part);
	return moved;
}

std::set<BodyPart> movedBy(const Pose& changed, const HumanModel& model = HumanModel{})
{
	return movedParts(drawModel(model, Pose{}), drawModel(model, changed));
}

float axisLength(const Mat4& m, const Vec3& axis)
{
	return m.transformDirection(axis).length();
}

Vec3 extents(const Mat4& m)
{
	return {axisLength(m, {1, 0, 0}), axisLength(m, {0, 1, 0}), axisLength(m, {0, 0, 1})};
}

bool isSegment(BodyPart p)
{
	switch (p)
	{
		case BodyPart::UpperArmLeft: case BodyPart::UpperArmRight:
		case BodyPart::ForearmLeft: case BodyPart::ForearmRight:
		case BodyPart::ThighLeft: case BodyPart::ThighRight:
		case BodyPart::LowerLegLeft: case BodyPart::LowerLegRight:
		case BodyPart::Neck:
		case BodyPart::HandLeft: case BodyPart::HandRight:
			return true;
		default:
			return false;
	}
}

float topY(const RecordingDrawer& d, BodyPart p) { return d.matrix(p).transformPoint({0, 0.5f, 0}).y; }
float bottomY(const RecordingDrawer& d, BodyPart p) { return d.matrix(p).transformPoint({0, -0.5f, 0}).y; }

} // namespace

TEST(HumanModel, DrawsEachOfFifteenPartsExactlyOnceWithBonusParts)
{
	const RecordingDrawer d = drawModel(HumanModel{}, Pose{});
	EXPECT_EQ(d.calls.size(), 15u);
	for (BodyPart part : allParts())
		EXPECT_EQ(d.count(part), 1) << bodyPartName(part);
}

TEST(HumanModel, DrawsTenPartsWithoutBonusParts)
{
	HumanModel model;
	model.setBonusPartsEnabled(false);
	const RecordingDrawer d = drawModel(model, Pose{});
	EXPECT_EQ(d.calls.size(), 10u);
	for (BodyPart part : allParts())
		EXPECT_EQ(d.count(part), isBonusPart(part) ? 0 : 1) << bodyPartName(part);
}

TEST(HumanModel, EachPartIsDrawnOnceEvenWithAnExtremePose)
{
	Pose p;
	p.rootYaw = 90.0f;
	p.leftArm = {170.0f, 90.0f, 140.0f, 40.0f};
	p.rightLeg = {100.0f, 30.0f, 140.0f, -30.0f};
	const RecordingDrawer d = drawModel(HumanModel{}, p);
	EXPECT_EQ(d.calls.size(), 15u);
	for (BodyPart part : allParts())
		EXPECT_EQ(d.count(part), 1) << bodyPartName(part);
}

TEST(HumanModel, IsVisibleFollowsBonusSwitch)
{
	HumanModel model;
	EXPECT_TRUE(model.isVisible(BodyPart::HandLeft));
	model.setBonusPartsEnabled(false);
	EXPECT_FALSE(model.isVisible(BodyPart::HandLeft));
	EXPECT_FALSE(model.isVisible(BodyPart::Neck));
	EXPECT_TRUE(model.isVisible(BodyPart::Torso));
	EXPECT_TRUE(model.isVisible(BodyPart::ForearmLeft));
}

TEST(HumanModel, TorsoAndHeadMatricesScaleUnitCubeToPartSize)
{
	const HumanModel model;
	const RecordingDrawer d = drawModel(model, Pose{});
	for (BodyPart part : {BodyPart::Torso, BodyPart::Head, BodyPart::FootLeft, BodyPart::FootRight})
		expectVecNear(extents(d.matrix(part)), model.dimensions().size(part));
}

TEST(HumanModel, LimbMatricesKeepWidthAndDepthOfThePart)
{
	const HumanModel model;
	const RecordingDrawer d = drawModel(model, Pose{});
	for (BodyPart part : allParts())
	{
		if (!isSegment(part))
			continue;
		const Vec3& size = model.dimensions().size(part);
		const Vec3 e = extents(d.matrix(part));
		EXPECT_NEAR(e.x, size.x, EPS) << bodyPartName(part);
		EXPECT_NEAR(e.z, size.z, EPS) << bodyPartName(part);
	}
}

TEST(HumanModel, LimbLengthCoversBoneAndAtMostRoundedJointOverlap)
{
	const HumanModel model;
	const RecordingDrawer d = drawModel(model, Pose{});
	for (BodyPart part : allParts())
	{
		if (!isSegment(part))
			continue;
		const Vec3& size = model.dimensions().size(part);
		const float thickness = std::min(size.x, size.z);
		const float length = extents(d.matrix(part)).y;
		EXPECT_GE(length, size.y - EPS) << bodyPartName(part);
		EXPECT_LE(length, size.y + thickness + EPS) << bodyPartName(part);
	}
}

TEST(HumanModel, MatricesAreRigidPlusScaleWithoutShear)
{
	Pose p;
	p.rootYaw = 40.0f;
	p.leftArm = {60.0f, 20.0f, 50.0f, 10.0f};
	const RecordingDrawer d = drawModel(HumanModel{}, p);
	for (const auto& call : d.calls)
	{
		const Vec3 x = call.matrix.transformDirection({1, 0, 0});
		const Vec3 y = call.matrix.transformDirection({0, 1, 0});
		const Vec3 z = call.matrix.transformDirection({0, 0, 1});
		EXPECT_NEAR(x.dot(y), 0.0f, EPS) << bodyPartName(call.part);
		EXPECT_NEAR(y.dot(z), 0.0f, EPS) << bodyPartName(call.part);
		EXPECT_NEAR(x.dot(z), 0.0f, EPS) << bodyPartName(call.part);
	}
}

TEST(HumanModel, TorsoHangsBelowTheShoulderLine)
{
	const HumanModel model;
	const RecordingDrawer d = drawModel(model, Pose{});
	EXPECT_NEAR(topY(d, BodyPart::Torso), 0.0f, EPS);
	EXPECT_NEAR(bottomY(d, BodyPart::Torso), -model.dimensions().size(BodyPart::Torso).y, EPS);
}

TEST(HumanModel, LeftSideIsOnPositiveX)
{
	const RecordingDrawer d = drawModel(HumanModel{}, Pose{});
	EXPECT_GT(d.center(BodyPart::UpperArmLeft).x, 0.0f);
	EXPECT_LT(d.center(BodyPart::UpperArmRight).x, 0.0f);
	EXPECT_GT(d.center(BodyPart::ThighLeft).x, 0.0f);
	EXPECT_LT(d.center(BodyPart::ThighRight).x, 0.0f);
}

TEST(HumanModel, DefaultPoseIsMirrorSymmetric)
{
	const RecordingDrawer d = drawModel(HumanModel{}, Pose{});
	const std::pair<BodyPart, BodyPart> pairs[] = {
		{BodyPart::UpperArmLeft, BodyPart::UpperArmRight}, {BodyPart::ForearmLeft, BodyPart::ForearmRight},
		{BodyPart::HandLeft, BodyPart::HandRight}, {BodyPart::ThighLeft, BodyPart::ThighRight},
		{BodyPart::LowerLegLeft, BodyPart::LowerLegRight}, {BodyPart::FootLeft, BodyPart::FootRight}};
	for (const auto& [l, r] : pairs)
	{
		const Vec3 cl = d.center(l);
		const Vec3 cr = d.center(r);
		expectVecNear(cl, {-cr.x, cr.y, cr.z});
	}
}

TEST(HumanModel, TorsoYawMovesEveryPart)
{
	Pose p;
	p.rootYaw = 35.0f;
	EXPECT_EQ(movedBy(p).size(), BODY_PART_COUNT);
}

TEST(HumanModel, TorsoPitchMovesEveryPart)
{
	Pose p;
	p.rootPitch = 25.0f;
	EXPECT_EQ(movedBy(p).size(), BODY_PART_COUNT);
}

TEST(HumanModel, TorsoRollMovesEveryPart)
{
	Pose p;
	p.rootRoll = 15.0f;
	EXPECT_EQ(movedBy(p).size(), BODY_PART_COUNT);
}

TEST(HumanModel, RootOffsetTranslatesEveryPartRigidly)
{
	Pose p;
	p.rootOffset = {1.0f, 2.0f, 3.0f};
	const RecordingDrawer a = drawModel(HumanModel{}, Pose{});
	const RecordingDrawer b = drawModel(HumanModel{}, p);
	for (BodyPart part : allParts())
		expectVecNear(b.center(part) - a.center(part), p.rootOffset);
}

TEST(HumanModel, LeftShoulderFlexMovesOnlyLeftArmChain)
{
	Pose p;
	p.leftArm.shoulderFlex = 50.0f;
	const std::set<BodyPart> expected = {BodyPart::UpperArmLeft, BodyPart::ForearmLeft, BodyPart::HandLeft};
	EXPECT_EQ(movedBy(p), expected);
}

TEST(HumanModel, RightShoulderAbductionMovesOnlyRightArmChain)
{
	Pose p;
	p.rightArm.shoulderAbduct = 40.0f;
	const std::set<BodyPart> expected = {BodyPart::UpperArmRight, BodyPart::ForearmRight, BodyPart::HandRight};
	EXPECT_EQ(movedBy(p), expected);
}

TEST(HumanModel, ElbowFlexMovesForearmAndHandButNotUpperArm)
{
	Pose p;
	p.leftArm.elbowFlex = 70.0f;
	const std::set<BodyPart> expected = {BodyPart::ForearmLeft, BodyPart::HandLeft};
	EXPECT_EQ(movedBy(p), expected);
}

TEST(HumanModel, WristFlexMovesOnlyTheHand)
{
	Pose p;
	p.rightArm.wristFlex = 40.0f;
	const std::set<BodyPart> expected = {BodyPart::HandRight};
	EXPECT_EQ(movedBy(p), expected);
}

TEST(HumanModel, HipFlexMovesWholeLegButNothingElse)
{
	Pose p;
	p.rightLeg.hipFlex = 45.0f;
	const std::set<BodyPart> expected = {BodyPart::ThighRight, BodyPart::LowerLegRight, BodyPart::FootRight};
	EXPECT_EQ(movedBy(p), expected);
}

TEST(HumanModel, KneeFlexMovesLowerLegAndFootButNotThigh)
{
	Pose p;
	p.leftLeg.kneeFlex = 60.0f;
	const std::set<BodyPart> expected = {BodyPart::LowerLegLeft, BodyPart::FootLeft};
	EXPECT_EQ(movedBy(p), expected);
}

TEST(HumanModel, AnkleFlexMovesOnlyTheFoot)
{
	Pose p;
	p.leftLeg.ankleFlex = 20.0f;
	const std::set<BodyPart> expected = {BodyPart::FootLeft};
	EXPECT_EQ(movedBy(p), expected);
}

TEST(HumanModel, NeckRotationMovesNeckAndHeadOnly)
{
	Pose p;
	p.neckYaw = 30.0f;
	const std::set<BodyPart> moved = movedBy(p);
	EXPECT_TRUE(moved.count(BodyPart::Head));
	EXPECT_TRUE(moved.count(BodyPart::Neck));
	EXPECT_EQ(moved.size(), 2u);
}

TEST(HumanModel, KneeFlexPositiveMovesFootBackward)
{
	Pose p;
	p.leftLeg.kneeFlex = 70.0f;
	const float before = drawModel(HumanModel{}, Pose{}).center(BodyPart::FootLeft).z;
	const float after = drawModel(HumanModel{}, p).center(BodyPart::FootLeft).z;
	EXPECT_LT(after, before - 0.1f);
}

TEST(HumanModel, KneeFlexPositiveRaisesTheFoot)
{
	Pose p;
	p.leftLeg.kneeFlex = 70.0f;
	const float before = drawModel(HumanModel{}, Pose{}).center(BodyPart::FootLeft).y;
	const float after = drawModel(HumanModel{}, p).center(BodyPart::FootLeft).y;
	EXPECT_GT(after, before);
}

TEST(HumanModel, ElbowFlexPositiveMovesHandForward)
{
	Pose p;
	p.rightArm.elbowFlex = 70.0f;
	const float before = drawModel(HumanModel{}, Pose{}).center(BodyPart::HandRight).z;
	const float after = drawModel(HumanModel{}, p).center(BodyPart::HandRight).z;
	EXPECT_GT(after, before + 0.1f);
}

TEST(HumanModel, ShoulderFlexPositiveSwingsArmForward)
{
	Pose p;
	p.leftArm.shoulderFlex = 90.0f;
	const RecordingDrawer d = drawModel(HumanModel{}, p);
	EXPECT_GT(d.center(BodyPart::HandLeft).z, 0.3f);
}

TEST(HumanModel, HipFlexPositiveSwingsLegForward)
{
	Pose p;
	p.leftLeg.hipFlex = 60.0f;
	const float before = drawModel(HumanModel{}, Pose{}).center(BodyPart::FootLeft).z;
	EXPECT_GT(drawModel(HumanModel{}, p).center(BodyPart::FootLeft).z, before + 0.2f);
}

TEST(HumanModel, ShoulderAbductionMovesHandAwayFromBody)
{
	Pose p;
	p.leftArm.shoulderAbduct = 60.0f;
	const float before = drawModel(HumanModel{}, Pose{}).center(BodyPart::HandLeft).x;
	EXPECT_GT(drawModel(HumanModel{}, p).center(BodyPart::HandLeft).x, before + 0.1f);
	Pose q;
	q.rightArm.shoulderAbduct = 60.0f;
	const float beforeR = drawModel(HumanModel{}, Pose{}).center(BodyPart::HandRight).x;
	EXPECT_LT(drawModel(HumanModel{}, q).center(BodyPart::HandRight).x, beforeR - 0.1f);
}

TEST(HumanModel, LongerUpperArmRepositionsForearmAndHand)
{
	HumanModel longer;
	Vec3 size = longer.dimensions().size(BodyPart::UpperArmLeft);
	const float growth = 0.3f;
	size.y += growth;
	longer.dimensions().setSize(BodyPart::UpperArmLeft, size);

	const RecordingDrawer a = drawModel(HumanModel{}, Pose{});
	const RecordingDrawer b = drawModel(longer, Pose{});
	EXPECT_NEAR(b.center(BodyPart::ForearmLeft).y, a.center(BodyPart::ForearmLeft).y - growth, EPS);
	EXPECT_NEAR(b.center(BodyPart::HandLeft).y, a.center(BodyPart::HandLeft).y - growth, EPS);
}

TEST(HumanModel, LongerUpperArmStaysAttachedAtTheShoulder)
{
	HumanModel longer;
	longer.dimensions().setSize(BodyPart::UpperArmLeft, {0.11f, 0.8f, 0.11f});
	const RecordingDrawer a = drawModel(HumanModel{}, Pose{});
	const RecordingDrawer b = drawModel(longer, Pose{});
	expectVecNear(b.matrix(BodyPart::UpperArmLeft).transformPoint({0, 0.5f, 0}),
				  a.matrix(BodyPart::UpperArmLeft).transformPoint({0, 0.5f, 0}));
}

TEST(HumanModel, UpperArmScaleDoesNotLeakToForearm)
{
	HumanModel bigger;
	bigger.dimensions().setSize(BodyPart::UpperArmLeft, {0.4f, 0.9f, 0.5f});
	const RecordingDrawer a = drawModel(HumanModel{}, Pose{});
	const RecordingDrawer b = drawModel(bigger, Pose{});
	expectVecNear(extents(b.matrix(BodyPart::ForearmLeft)), extents(a.matrix(BodyPart::ForearmLeft)));
	expectVecNear(extents(b.matrix(BodyPart::HandLeft)), extents(a.matrix(BodyPart::HandLeft)));
}

TEST(HumanModel, TorsoScaleDoesNotLeakToChildren)
{
	HumanModel bigger;
	bigger.dimensions().setSize(BodyPart::Torso, {0.4f, 1.2f, 0.6f});
	const RecordingDrawer a = drawModel(HumanModel{}, Pose{});
	const RecordingDrawer b = drawModel(bigger, Pose{});
	for (BodyPart part : {BodyPart::Head, BodyPart::UpperArmLeft, BodyPart::ThighRight, BodyPart::FootLeft})
		expectVecNear(extents(b.matrix(part)), extents(a.matrix(part)));
}

TEST(HumanModel, LongerThighLowersLowerLegAndFoot)
{
	HumanModel longer;
	longer.dimensions().setSize(BodyPart::ThighLeft, {0.15f, 0.84f, 0.16f});
	const RecordingDrawer a = drawModel(HumanModel{}, Pose{});
	const RecordingDrawer b = drawModel(longer, Pose{});
	EXPECT_NEAR(b.center(BodyPart::LowerLegLeft).y, a.center(BodyPart::LowerLegLeft).y - 0.4f, EPS);
	EXPECT_NEAR(b.center(BodyPart::FootLeft).y, a.center(BodyPart::FootLeft).y - 0.4f, EPS);
	// The other leg is untouched.
	expectVecNear(b.center(BodyPart::FootRight), a.center(BodyPart::FootRight));
}

TEST(HumanModel, TallerTorsoLowersLegs)
{
	HumanModel taller;
	taller.dimensions().setSize(BodyPart::Torso, {0.4f, 1.0f, 0.22f});
	const RecordingDrawer a = drawModel(HumanModel{}, Pose{});
	const RecordingDrawer b = drawModel(taller, Pose{});
	for (BodyPart part : {BodyPart::ThighLeft, BodyPart::ThighRight, BodyPart::FootLeft, BodyPart::FootRight})
		EXPECT_NEAR(b.center(part).y, a.center(part).y - 0.4f, EPS) << bodyPartName(part);
}

TEST(HumanModel, TallerTorsoKeepsTopAtShoulderLineAndExtendsDownward)
{
	HumanModel taller;
	taller.dimensions().setSize(BodyPart::Torso, {0.4f, 1.0f, 0.22f});
	const RecordingDrawer b = drawModel(taller, Pose{});
	EXPECT_NEAR(topY(b, BodyPart::Torso), 0.0f, EPS);
	EXPECT_NEAR(bottomY(b, BodyPart::Torso), -1.0f, EPS);
}

TEST(HumanModel, TallerTorsoKeepsHeadAndArmsAttachedAtTheShoulders)
{
	HumanModel taller;
	taller.dimensions().setSize(BodyPart::Torso, {0.4f, 1.0f, 0.22f});
	const RecordingDrawer a = drawModel(HumanModel{}, Pose{});
	const RecordingDrawer b = drawModel(taller, Pose{});
	expectVecNear(b.center(BodyPart::Head), a.center(BodyPart::Head));
	expectVecNear(b.center(BodyPart::UpperArmLeft), a.center(BodyPart::UpperArmLeft));
}

TEST(HumanModel, LegsStayGluedToTheBottomOfTheTorsoWhenItChanges)
{
	HumanModel taller;
	taller.dimensions().setSize(BodyPart::Torso, {0.4f, 0.9f, 0.22f});
	const RecordingDrawer d = drawModel(taller, Pose{});
	EXPECT_NEAR(bottomY(d, BodyPart::Torso), -0.9f, EPS);
	EXPECT_NEAR(topY(d, BodyPart::ThighLeft), -0.9f + std::min(0.15f, 0.16f) / 2.0f, EPS);
}

TEST(HumanModel, WiderTorsoMovesArmsSideways)
{
	HumanModel wider;
	wider.dimensions().setSize(BodyPart::Torso, {0.8f, 0.6f, 0.22f});
	const RecordingDrawer a = drawModel(HumanModel{}, Pose{});
	const RecordingDrawer b = drawModel(wider, Pose{});
	EXPECT_NEAR(b.center(BodyPart::UpperArmLeft).x, a.center(BodyPart::UpperArmLeft).x + 0.2f, EPS);
	EXPECT_NEAR(b.center(BodyPart::UpperArmRight).x, a.center(BodyPart::UpperArmRight).x - 0.2f, EPS);
}

TEST(HumanModel, HeadSitsOnTopOfTheNeckWhenBonusPartsAreVisible)
{
	const HumanModel model;
	const RecordingDrawer d = drawModel(model, Pose{});
	EXPECT_NEAR(bottomY(d, BodyPart::Head), model.dimensions().size(BodyPart::Neck).y, EPS);
}

TEST(HumanModel, HeadStaysAttachedToTorsoWhenBonusPartsAreHidden)
{
	HumanModel model;
	model.setBonusPartsEnabled(false);
	const RecordingDrawer d = drawModel(model, Pose{});
	EXPECT_NEAR(bottomY(d, BodyPart::Head), topY(d, BodyPart::Torso), EPS);
}

TEST(HumanModel, HidingBonusPartsDoesNotChangeMainPartsOtherThanTheHead)
{
	HumanModel hidden;
	hidden.setBonusPartsEnabled(false);
	const RecordingDrawer a = drawModel(HumanModel{}, Pose{});
	const RecordingDrawer b = drawModel(hidden, Pose{});
	EXPECT_TRUE(sameMatrix(a.matrix(BodyPart::Torso), b.matrix(BodyPart::Torso)));
	EXPECT_TRUE(sameMatrix(a.matrix(BodyPart::UpperArmLeft), b.matrix(BodyPart::UpperArmLeft)));
	EXPECT_TRUE(sameMatrix(a.matrix(BodyPart::ThighRight), b.matrix(BodyPart::ThighRight)));
}

TEST(HumanModel, ForearmStaysAttachedToUpperArmWhenHandIsHidden)
{
	HumanModel hidden;
	hidden.setBonusPartsEnabled(false);
	Pose p;
	p.leftArm = {40.0f, 10.0f, 60.0f, 0.0f};
	const RecordingDrawer a = drawModel(HumanModel{}, p);
	const RecordingDrawer b = drawModel(hidden, p);
	expectVecNear(a.matrix(BodyPart::ForearmLeft).transformPoint({0, 0.5f, 0}),
				  b.matrix(BodyPart::ForearmLeft).transformPoint({0, 0.5f, 0}));
}

TEST(HumanModel, MatrixStackDepthIsRestoredAfterDraw)
{
	MatrixStack stack;
	stack.push();
	stack.translate({1, 2, 3});
	const std::size_t before = stack.depth();
	const Mat4 topBefore = stack.top();
	RecordingDrawer drawer;
	Pose p;
	p.leftArm = {30, 10, 40, 5};
	p.rightLeg = {20, 3, 50, -5};
	HumanModel{}.draw(stack, p, drawer);
	EXPECT_EQ(stack.depth(), before);
	EXPECT_TRUE(sameMatrix(stack.top(), topBefore));
}

TEST(HumanModel, MatrixStackDepthIsRestoredWithoutBonusParts)
{
	HumanModel model;
	model.setBonusPartsEnabled(false);
	MatrixStack stack;
	RecordingDrawer drawer;
	model.draw(stack, Pose{}, drawer);
	EXPECT_EQ(stack.depth(), 1u);
}

TEST(HumanModel, DrawingUnderAnExistingTransformMovesTheWholeBody)
{
	MatrixStack stack;
	stack.translate({5.0f, 0.0f, 0.0f});
	RecordingDrawer shifted;
	HumanModel{}.draw(stack, Pose{}, shifted);
	const RecordingDrawer plain = drawModel(HumanModel{}, Pose{});
	for (BodyPart part : allParts())
		expectVecNear(shifted.center(part) - plain.center(part), {5.0f, 0.0f, 0.0f});
}

// ----------------------------------------------- roll before pitch order ---

namespace {
// Lies the body face down (pitch 90) and rolls it by `roll`.
Pose lyingPose(float roll)
{
	Pose p;
	p.rootPitch = 90.0f;
	p.rootRoll = roll;
	return p;
}

Vec3 torsoAxis(const RecordingDrawer& d)
{
	return d.matrix(BodyPart::Torso).transformDirection({0, 1, 0}).normalized();
}
}

TEST(HumanModelRoot, PitchNinetyLaysTheBodyHorizontal)
{
	const RecordingDrawer d = drawModel(HumanModel{}, lyingPose(0.0f));
	EXPECT_NEAR(torsoAxis(d).y, 0.0f, EPS);
	EXPECT_NEAR(d.center(BodyPart::Head).y, 0.0f, EPS);
}

TEST(HumanModelRoot, RollAfterPitchNinetyKeepsTheTorsoAxisHorizontal)
{
	const RecordingDrawer d = drawModel(HumanModel{}, lyingPose(30.0f));
	EXPECT_NEAR(torsoAxis(d).y, 0.0f, EPS);
}

TEST(HumanModelRoot, RollAfterPitchNinetyKeepsHeadAtTheTorsoAxisHeight)
{
	const RecordingDrawer d = drawModel(HumanModel{}, lyingPose(30.0f));
	EXPECT_NEAR(d.center(BodyPart::Head).y, 0.0f, EPS);
	EXPECT_NEAR(d.center(BodyPart::Torso).y, 0.0f, EPS);
}

TEST(HumanModelRoot, RollWhileLyingTurnsTheBodyAroundItsLongAxis)
{
	const RecordingDrawer flat = drawModel(HumanModel{}, lyingPose(0.0f));
	const RecordingDrawer rolled = drawModel(HumanModel{}, lyingPose(30.0f));
	// Shoulders were level; they now differ in height, symmetrically.
	EXPECT_NEAR(flat.center(BodyPart::UpperArmLeft).y, flat.center(BodyPart::UpperArmRight).y, EPS);
	const float left = rolled.center(BodyPart::UpperArmLeft).y;
	const float right = rolled.center(BodyPart::UpperArmRight).y;
	EXPECT_GT(std::abs(left - right), 0.1f);
	EXPECT_NEAR(left, -right, EPS);
}

TEST(HumanModelRoot, RollWhileLyingChangesShoulderHeightByTheExpectedAmount)
{
	const HumanModel model;
	const RecordingDrawer d = drawModel(model, lyingPose(30.0f));
	const float reach = model.dimensions().size(BodyPart::Torso).x / 2.0f
		+ model.dimensions().size(BodyPart::UpperArmLeft).x / 2.0f;
	EXPECT_NEAR(std::abs(d.center(BodyPart::UpperArmLeft).y - d.center(BodyPart::UpperArmRight).y),
				2.0f * reach * std::sin(math::radians(30.0f)), 2e-2f);
}

TEST(HumanModelRoot, PositiveRollWhileUprightLeansTowardTheLeftSide)
{
	Pose p;
	p.rootRoll = 20.0f;
	const RecordingDrawer d = drawModel(HumanModel{}, p);
	// The head tilts toward +X (left) and the feet swing the other way.
	EXPECT_GT(d.center(BodyPart::Head).x, 0.0f);
	EXPECT_LT(d.center(BodyPart::FootLeft).x, d.center(BodyPart::UpperArmLeft).x);
}

TEST(HumanModelRoot, UprightPitchLeansForward)
{
	Pose p;
	p.rootPitch = 30.0f;
	EXPECT_GT(drawModel(HumanModel{}, p).center(BodyPart::Head).z, 0.05f);
}

// ------------------------------------------------------------ jointOverlap ---

namespace {
float lowestCornerY(const Mat4& m)
{
	float lowest = 1e9f;
	for (const Vec3& c : cubeCorners())
		lowest = std::min(lowest, m.transformPoint(c).y);
	return lowest;
}

Pose verticalArmWithWrist(float wrist)
{
	Pose p;
	p.leftArm.wristFlex = wrist;
	return p;
}
}

TEST(HumanModelOverlap, ForearmEndNeverGoesBelowTheHandWhenWristIsFlexed)
{
	const HumanModel model;
	for (float wrist : {-80.0f, -40.0f, 0.0f, 40.0f, 80.0f})
	{
		const RecordingDrawer d = drawModel(model, verticalArmWithWrist(wrist));
		EXPECT_GE(lowestCornerY(d.matrix(BodyPart::ForearmLeft)) + EPS, lowestCornerY(d.matrix(BodyPart::HandLeft)))
			<< wrist;
	}
}

TEST(HumanModelOverlap, HandIsTheLowestPartOfAVerticalArmWithWristFlexedBack)
{
	const RecordingDrawer d = drawModel(HumanModel{}, verticalArmWithWrist(-80.0f));
	float lowest = 1e9f;
	BodyPart lowestPart = BodyPart::Torso;
	for (BodyPart part : {BodyPart::UpperArmLeft, BodyPart::ForearmLeft, BodyPart::HandLeft})
	{
		const float y = lowestCornerY(d.matrix(part));
		if (y < lowest)
		{
			lowest = y;
			lowestPart = part;
		}
	}
	EXPECT_EQ(lowestPart, BodyPart::HandLeft);
}

TEST(HumanModelOverlap, ForearmEndStaysInsideTheHandWhenAHandIsDrawn)
{
	const HumanModel model;
	const RecordingDrawer d = drawModel(model, Pose{});
	const Vec3& fore = model.dimensions().size(BodyPart::ForearmLeft);
	const Vec3& hand = model.dimensions().size(BodyPart::HandLeft);
	const float overlap = std::min({std::min(fore.x, fore.z), hand.x, hand.y, hand.z}) / 2.0f;
	const float wristY = d.matrix(BodyPart::HandLeft).transformPoint({0, 0.5f, 0}).y
		- std::min(hand.x, hand.z) / 2.0f;
	EXPECT_NEAR(bottomY(d, BodyPart::ForearmLeft), wristY - overlap, 1e-3f);
}

TEST(HumanModelOverlap, ForearmEndIsRoundedByHalfItsThicknessWhenBonusPartsAreHidden)
{
	HumanModel model;
	model.setBonusPartsEnabled(false);
	const RecordingDrawer d = drawModel(model, Pose{});
	const Vec3& fore = model.dimensions().size(BodyPart::ForearmLeft);
	const float thickness = std::min(fore.x, fore.z);
	const float topOfForearm = topY(d, BodyPart::ForearmLeft);
	EXPECT_NEAR(topOfForearm - bottomY(d, BodyPart::ForearmLeft), fore.y + thickness, 1e-3f);
}

TEST(HumanModelOverlap, HiddenShinEndIsRoundedOffToo)
{
	HumanModel model;
	model.setBonusPartsEnabled(false);
	const RecordingDrawer d = drawModel(model, Pose{});
	const Vec3& shin = model.dimensions().size(BodyPart::LowerLegLeft);
	EXPECT_NEAR(extents(d.matrix(BodyPart::LowerLegLeft)).y, shin.y + std::min(shin.x, shin.z) / 2.0f * 2.0f, 1e-3f);
}

TEST(HumanModelOverlap, HandsHaveNoDistalOverlap)
{
	const HumanModel model;
	const RecordingDrawer d = drawModel(model, Pose{});
	const Vec3& hand = model.dimensions().size(BodyPart::HandLeft);
	const float thickness = std::min(hand.x, hand.z);
	EXPECT_NEAR(extents(d.matrix(BodyPart::HandLeft)).y, hand.y + thickness / 2.0f, 1e-3f);
}

TEST(HumanModelOverlap, ThinChildLimitsTheOverlapOfAThickParent)
{
	HumanModel model;
	model.dimensions().setSize(BodyPart::HandLeft, {0.09f, 0.17f, 0.02f});
	const RecordingDrawer d = drawModel(model, Pose{});
	const Vec3& fore = model.dimensions().size(BodyPart::ForearmLeft);
	// distal overlap = min(forearm thickness, hand dims) / 2 = 0.01
	EXPECT_NEAR(extents(d.matrix(BodyPart::ForearmLeft)).y, fore.y + std::min(fore.x, fore.z) / 2.0f + 0.01f, 1e-3f);
}

// ------------------------------------------------- single draw call, shoulders ---

namespace {
std::vector<Pose> severalPoses()
{
	std::vector<Pose> poses(4);
	poses[1].rootYaw = 70.0f;
	poses[1].leftArm = {90.0f, 20.0f, 100.0f, -40.0f};
	poses[2].rootPitch = 80.0f;
	poses[2].rootRoll = 25.0f;
	poses[2].rightLeg = {80.0f, 10.0f, 120.0f, 20.0f};
	poses[3].neckYaw = 40.0f;
	poses[3].neckPitch = -30.0f;
	poses[3].leftLeg = {-20.0f, 0.0f, 10.0f, -30.0f};
	return poses;
}

float highestCornerY(const Mat4& m)
{
	float highest = -1e9f;
	for (const Vec3& c : cubeCorners())
		highest = std::max(highest, m.transformPoint(c).y);
	return highest;
}
}

TEST(HumanModelDrawCalls, EveryVisiblePartIsDrawnExactlyOnceForSeveralPoses)
{
	for (bool bonus : {true, false})
	{
		HumanModel model;
		model.setBonusPartsEnabled(bonus);
		for (const Pose& pose : severalPoses())
		{
			const RecordingDrawer d = drawModel(model, pose);
			for (BodyPart part : allParts())
				EXPECT_EQ(d.count(part), model.isVisible(part) ? 1 : 0) << bodyPartName(part) << " bonus=" << bonus;
			EXPECT_EQ(d.calls.size(), bonus ? 15u : 10u);
		}
	}
}

TEST(HumanModelDrawCalls, EveryVisiblePartIsDrawnOnceAfterResizing)
{
	HumanModel model;
	model.dimensions().setSize(BodyPart::Torso, {0.8f, 1.3f, 0.5f});
	model.dimensions().setSize(BodyPart::ForearmLeft, {0.05f, 0.9f, 0.3f});
	const RecordingDrawer d = drawModel(model, severalPoses()[1]);
	EXPECT_EQ(d.calls.size(), 15u);
	for (BodyPart part : allParts())
		EXPECT_EQ(d.count(part), 1) << bodyPartName(part);
}

TEST(HumanModelShoulders, UpperArmTopIsLevelWithTorsoTop)
{
	const RecordingDrawer d = drawModel(HumanModel{}, Pose{});
	for (auto arm : {BodyPart::UpperArmLeft, BodyPart::UpperArmRight})
		EXPECT_NEAR(highestCornerY(d.matrix(arm)), highestCornerY(d.matrix(BodyPart::Torso)), EPS) << bodyPartName(arm);
}

TEST(HumanModelShoulders, UpperArmTopStaysLevelAfterResizingTheUpperArm)
{
	HumanModel model;
	model.dimensions().setSize(BodyPart::UpperArmLeft, {0.3f, 0.5f, 0.2f});
	model.dimensions().setSize(BodyPart::UpperArmRight, {0.07f, 0.5f, 0.2f});
	const RecordingDrawer d = drawModel(model, Pose{});
	for (auto arm : {BodyPart::UpperArmLeft, BodyPart::UpperArmRight})
		EXPECT_NEAR(highestCornerY(d.matrix(arm)), highestCornerY(d.matrix(BodyPart::Torso)), EPS) << bodyPartName(arm);
}

TEST(HumanModelShoulders, UpperArmTopStaysLevelAfterResizingTheTorso)
{
	HumanModel model;
	model.dimensions().setSize(BodyPart::Torso, {0.9f, 1.2f, 0.4f});
	const RecordingDrawer d = drawModel(model, Pose{});
	for (auto arm : {BodyPart::UpperArmLeft, BodyPart::UpperArmRight})
		EXPECT_NEAR(highestCornerY(d.matrix(arm)), highestCornerY(d.matrix(BodyPart::Torso)), EPS) << bodyPartName(arm);
}

// ----------------------------------------------------------------------------
// Rotation order at shoulders and hips: flexion swings the abducted limb, so
// abduction still works when the limb is flexed.

namespace {

Vec3 handCenter(const Pose& pose, BodyPart hand = BodyPart::HandLeft)
{
	return drawModel(HumanModel{}, pose).center(hand);
}

Vec3 footCenter(const Pose& pose, BodyPart foot = BodyPart::FootLeft)
{
	return drawModel(HumanModel{}, pose).center(foot);
}

Pose armPose(Side side, float flex, float abduct)
{
	Pose pose;
	ArmPose& arm = side == Side::Left ? pose.leftArm : pose.rightArm;
	arm.shoulderFlex = flex;
	arm.shoulderAbduct = abduct;
	return pose;
}

Pose legPose(Side side, float flex, float abduct)
{
	Pose pose;
	LegPose& leg = side == Side::Left ? pose.leftLeg : pose.rightLeg;
	leg.hipFlex = flex;
	leg.hipAbduct = abduct;
	return pose;
}

}

TEST(HumanModelRotationOrder, FlexedArmPutsTheHandInFrontOfTheShoulder)
{
	const Vec3 hand = handCenter(armPose(Side::Left, 90.0f, 0.0f));
	const Vec3 rest = handCenter(Pose{});
	EXPECT_GT(hand.z, rest.z + 0.5f);
	EXPECT_NEAR(hand.x, rest.x, 1e-3f);
}

TEST(HumanModelRotationOrder, AbductionStillMovesTheLeftHandOutwardWhenTheArmIsFlexed)
{
	const Vec3 straight = handCenter(armPose(Side::Left, 90.0f, 0.0f));
	const Vec3 out = handCenter(armPose(Side::Left, 90.0f, 30.0f));
	const Vec3 further = handCenter(armPose(Side::Left, 90.0f, 60.0f));
	EXPECT_GT(out.x, straight.x + 0.1f);
	EXPECT_GT(further.x, out.x + 0.1f);
}

TEST(HumanModelRotationOrder, AbductionStillMovesTheRightHandOutwardWhenTheArmIsFlexed)
{
	const Vec3 straight = handCenter(armPose(Side::Right, 90.0f, 0.0f), BodyPart::HandRight);
	const Vec3 out = handCenter(armPose(Side::Right, 90.0f, 30.0f), BodyPart::HandRight);
	EXPECT_LT(out.x, straight.x - 0.1f);
}

TEST(HumanModelRotationOrder, NegativeAbductionMovesTheFlexedHandTowardTheMidline)
{
	const Vec3 straight = handCenter(armPose(Side::Left, 90.0f, 0.0f));
	const Vec3 inward = handCenter(armPose(Side::Left, 90.0f, -25.0f));
	EXPECT_LT(inward.x, straight.x - 0.05f);
	EXPECT_GT(inward.z, straight.z - 0.5f);	// the hand is still in front
}

TEST(HumanModelRotationOrder, FlexedAbductedHandStaysAtShoulderHeight)
{
	const Vec3 straight = handCenter(armPose(Side::Left, 90.0f, 0.0f));
	const Vec3 out = handCenter(armPose(Side::Left, 90.0f, 45.0f));
	EXPECT_NEAR(out.y, straight.y, 1e-3f);
}

TEST(HumanModelRotationOrder, TPoseWithoutFlexionPutsTheHandOutAtShoulderHeight)
{
	const HumanModel model;
	const float shoulderY = -std::min(model.dimensions().size(BodyPart::UpperArmLeft).x,
									 model.dimensions().size(BodyPart::UpperArmLeft).z) / 2.0f;
	const Vec3 hand = handCenter(armPose(Side::Left, 0.0f, 90.0f));
	const Vec3 rest = handCenter(Pose{});
	EXPECT_NEAR(hand.y, shoulderY, 1e-3f);
	EXPECT_NEAR(hand.z, 0.0f, 1e-3f);
	EXPECT_GT(hand.x, rest.x + 0.5f);
}

TEST(HumanModelRotationOrder, AbductionStillMovesTheFootOutwardWhenTheLegIsFlexed)
{
	const Vec3 straight = footCenter(legPose(Side::Left, 90.0f, 0.0f));
	const Vec3 out = footCenter(legPose(Side::Left, 90.0f, 30.0f));
	const Vec3 in = footCenter(legPose(Side::Left, 90.0f, -20.0f));
	EXPECT_GT(straight.z, footCenter(Pose{}).z + 0.5f);
	EXPECT_GT(out.x, straight.x + 0.1f);
	EXPECT_LT(in.x, straight.x - 0.05f);
}

TEST(HumanModelRotationOrder, RightFootAbductsOutwardWhenTheLegIsFlexed)
{
	const Vec3 straight = footCenter(legPose(Side::Right, 90.0f, 0.0f), BodyPart::FootRight);
	const Vec3 out = footCenter(legPose(Side::Right, 90.0f, 30.0f), BodyPart::FootRight);
	EXPECT_LT(out.x, straight.x - 0.1f);
}

TEST(HumanModelRotationOrder, LegWithoutFlexionAbductsSideways)
{
	const Vec3 hand = footCenter(legPose(Side::Left, 0.0f, 90.0f));
	const Vec3 rest = footCenter(Pose{});
	EXPECT_GT(hand.x, rest.x + 0.5f);
	EXPECT_NEAR(hand.z, rest.z, 0.2f);
}
