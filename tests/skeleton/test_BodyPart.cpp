#include <gtest/gtest.h>

#include "../support/DrawHelpers.hpp"

#include "render/BodyPalette.hpp"

#include <set>
#include <string>

using namespace testing_support;

namespace {

BodyPart partAt(std::size_t i) { return static_cast<BodyPart>(i); }

bool isLeft(BodyPart p)
{
	return p == BodyPart::UpperArmLeft || p == BodyPart::ForearmLeft || p == BodyPart::HandLeft
		|| p == BodyPart::ThighLeft || p == BodyPart::LowerLegLeft || p == BodyPart::FootLeft;
}

bool isCentral(BodyPart p)
{
	return p == BodyPart::Torso || p == BodyPart::Head || p == BodyPart::Neck;
}

void expectSameSizes(const BodyDimensions& a, const BodyDimensions& b)
{
	for (std::size_t i = 0; i < BODY_PART_COUNT; ++i)
		expectVecNear(a.size(partAt(i)), b.size(partAt(i)), 0.0f);
}

void expectMirrored(const RecordingDrawer& d, BodyPart left)
{
	const Vec3 l = d.center(left);
	const Vec3 r = d.center(mirrorPart(left));
	EXPECT_NEAR(l.x, -r.x, 1e-4f) << bodyPartName(left);
	EXPECT_NEAR(l.y, r.y, 1e-4f) << bodyPartName(left);
	EXPECT_NEAR(l.z, r.z, 1e-4f) << bodyPartName(left);
}

} // namespace

TEST(BodyPart, MirrorIsAnInvolution)
{
	for (std::size_t i = 0; i < BODY_PART_COUNT; ++i)
		EXPECT_EQ(mirrorPart(mirrorPart(partAt(i))), partAt(i)) << bodyPartName(partAt(i));
}

TEST(BodyPart, CentralPartsMirrorToThemselves)
{
	for (BodyPart p : {BodyPart::Torso, BodyPart::Head, BodyPart::Neck})
		EXPECT_EQ(mirrorPart(p), p);
}

TEST(BodyPart, SidedPartsMirrorToTheOtherSideWithSameSize)
{
	const BodyDimensions dims;
	int leftCount = 0;
	for (std::size_t i = 0; i < BODY_PART_COUNT; ++i)
	{
		const BodyPart p = partAt(i);
		if (isCentral(p))
			continue;
		const BodyPart m = mirrorPart(p);
		EXPECT_NE(m, p) << bodyPartName(p);
		EXPECT_NE(isLeft(p), isLeft(m)) << bodyPartName(p);
		expectVecNear(dims.size(p), dims.size(m), 0.0f);
		if (isLeft(p))
			++leftCount;
	}
	EXPECT_EQ(leftCount, 6);
}

TEST(BodyPart, MirrorIsABijection)
{
	std::set<BodyPart> images;
	for (std::size_t i = 0; i < BODY_PART_COUNT; ++i)
		images.insert(mirrorPart(partAt(i)));
	EXPECT_EQ(images.size(), BODY_PART_COUNT);
}

TEST(BodyPart, NamesAreDistinctAndNonEmpty)
{
	std::set<std::string> names;
	for (std::size_t i = 0; i < BODY_PART_COUNT; ++i)
	{
		const char* n = bodyPartName(partAt(i));
		ASSERT_NE(n, nullptr);
		EXPECT_GT(std::string(n).size(), 0u);
		names.insert(n);
	}
	EXPECT_EQ(names.size(), BODY_PART_COUNT);
	EXPECT_STREQ(bodyPartName(BodyPart::Count), "Unknown");
}

TEST(BodyPart, BonusPartsAreExactlyNeckHandsAndFeet)
{
	const std::set<BodyPart> bonus = {BodyPart::Neck, BodyPart::HandLeft, BodyPart::HandRight,
		BodyPart::FootLeft, BodyPart::FootRight};
	for (std::size_t i = 0; i < BODY_PART_COUNT; ++i)
		EXPECT_EQ(isBonusPart(partAt(i)), bonus.count(partAt(i)) == 1) << bodyPartName(partAt(i));
}

TEST(BodyPart, SideSigns)
{
	EXPECT_FLOAT_EQ(sideSign(Side::Left), 1.0f);
	EXPECT_FLOAT_EQ(sideSign(Side::Right), -1.0f);
}

TEST(BodyPartEditing, ResizingBothSidesKeepsModelSymmetric)
{
	HumanModel model;
	const std::vector<BodyPart> edited = {BodyPart::ForearmLeft, BodyPart::UpperArmLeft,
		BodyPart::ThighLeft, BodyPart::FootLeft};
	float k = 1.3f;
	for (BodyPart p : edited)
	{
		const Vec3 s = model.dimensions().size(p);
		const Vec3 resized{s.x * k, s.y * k, s.z * k};
		model.dimensions().setSize(p, resized);
		model.dimensions().setSize(mirrorPart(p), resized);
		k += 0.2f;
	}
	const RecordingDrawer d = drawModel(model, Pose{});
	for (std::size_t i = 0; i < BODY_PART_COUNT; ++i)
		if (isLeft(partAt(i)))
			expectMirrored(d, partAt(i));
	EXPECT_GT(d.center(BodyPart::HandLeft).x, 0.0f);
}

TEST(BodyPartEditing, EditingOneSideOnlyBreaksSymmetry)
{
	HumanModel model;
	const Vec3 s = model.dimensions().size(BodyPart::ForearmLeft);
	model.dimensions().setSize(BodyPart::ForearmLeft, {s.x, s.y * 2.0f, s.z});
	const RecordingDrawer d = drawModel(model, Pose{});
	EXPECT_GT(std::abs(d.center(BodyPart::HandLeft).y - d.center(BodyPart::HandRight).y), 0.1f);
}

TEST(BodyPartEditing, ResettingDimensionsRestoresDefaults)
{
	HumanModel model;
	for (std::size_t i = 0; i < BODY_PART_COUNT; ++i)
		model.dimensions().setSize(partAt(i), {2.0f, 3.0f, 4.0f});
	model.dimensions() = BodyDimensions();
	expectSameSizes(model.dimensions(), BodyDimensions());
	const RecordingDrawer after = drawModel(model, Pose{});
	const RecordingDrawer fresh = drawModel(HumanModel{}, Pose{});
	for (std::size_t i = 0; i < BODY_PART_COUNT; ++i)
		EXPECT_TRUE(sameMatrix(after.matrix(partAt(i)), fresh.matrix(partAt(i)))) << bodyPartName(partAt(i));
}

TEST(BodyPartEditing, ResettingPaletteRestoresDefaults)
{
	BodyPalette palette;
	for (std::size_t i = 0; i < BODY_PART_COUNT; ++i)
		palette.setColor(partAt(i), {0.123f, 0.456f, 0.789f});
	palette = BodyPalette();
	const BodyPalette fresh;
	for (std::size_t i = 0; i < BODY_PART_COUNT; ++i)
		expectVecNear(palette.color(partAt(i)), fresh.color(partAt(i)), 0.0f);
}
