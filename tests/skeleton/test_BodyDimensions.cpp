#include <gtest/gtest.h>

#include "skeleton/BodyDimensions.hpp"

namespace {
void expectMirrored(const BodyDimensions& d, BodyPart left, BodyPart right)
{
	EXPECT_FLOAT_EQ(d.size(left).x, d.size(right).x) << bodyPartName(left);
	EXPECT_FLOAT_EQ(d.size(left).y, d.size(right).y) << bodyPartName(left);
	EXPECT_FLOAT_EQ(d.size(left).z, d.size(right).z) << bodyPartName(left);
}
}

TEST(BodyDimensions, DefaultSizesAreAllPositive)
{
	const BodyDimensions d;
	for (std::size_t i = 0; i < BODY_PART_COUNT; ++i)
	{
		const Vec3& s = d.size(static_cast<BodyPart>(i));
		EXPECT_GT(s.x, 0.0f) << i;
		EXPECT_GT(s.y, 0.0f) << i;
		EXPECT_GT(s.z, 0.0f) << i;
	}
}

TEST(BodyDimensions, ArmsAreSymmetricByDefault)
{
	const BodyDimensions d;
	expectMirrored(d, BodyPart::UpperArmLeft, BodyPart::UpperArmRight);
	expectMirrored(d, BodyPart::ForearmLeft, BodyPart::ForearmRight);
	expectMirrored(d, BodyPart::HandLeft, BodyPart::HandRight);
}

TEST(BodyDimensions, LegsAreSymmetricByDefault)
{
	const BodyDimensions d;
	expectMirrored(d, BodyPart::ThighLeft, BodyPart::ThighRight);
	expectMirrored(d, BodyPart::LowerLegLeft, BodyPart::LowerLegRight);
	expectMirrored(d, BodyPart::FootLeft, BodyPart::FootRight);
}

TEST(BodyDimensions, SetSizeStoresValue)
{
	BodyDimensions d;
	d.setSize(BodyPart::Head, {0.5f, 0.6f, 0.7f});
	EXPECT_FLOAT_EQ(d.size(BodyPart::Head).x, 0.5f);
	EXPECT_FLOAT_EQ(d.size(BodyPart::Head).y, 0.6f);
	EXPECT_FLOAT_EQ(d.size(BodyPart::Head).z, 0.7f);
}

TEST(BodyDimensions, SetSizeOnlyChangesThatPart)
{
	BodyDimensions d;
	const Vec3 before = d.size(BodyPart::UpperArmRight);
	d.setSize(BodyPart::UpperArmLeft, {1.0f, 1.0f, 1.0f});
	EXPECT_FLOAT_EQ(d.size(BodyPart::UpperArmRight).y, before.y);
}

TEST(BodyDimensions, SetSizeClampsToMinimum)
{
	BodyDimensions d;
	d.setSize(BodyPart::Torso, {0.0f, -3.0f, 0.001f});
	EXPECT_FLOAT_EQ(d.size(BodyPart::Torso).x, BodyDimensions::MIN_SIZE);
	EXPECT_FLOAT_EQ(d.size(BodyPart::Torso).y, BodyDimensions::MIN_SIZE);
	EXPECT_FLOAT_EQ(d.size(BodyPart::Torso).z, BodyDimensions::MIN_SIZE);
}

TEST(BodyDimensions, SetSizeKeepsValuesAboveMinimum)
{
	BodyDimensions d;
	d.setSize(BodyPart::Torso, {BodyDimensions::MIN_SIZE, 2.0f, 0.3f});
	EXPECT_FLOAT_EQ(d.size(BodyPart::Torso).x, BodyDimensions::MIN_SIZE);
	EXPECT_FLOAT_EQ(d.size(BodyPart::Torso).y, 2.0f);
}
