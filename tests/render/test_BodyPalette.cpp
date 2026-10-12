#include <gtest/gtest.h>

#include "render/BodyPalette.hpp"

namespace {
BodyPart partAt(std::size_t i) { return static_cast<BodyPart>(i); }

void expectSameColor(const Vec3& a, const Vec3& b)
{
	EXPECT_FLOAT_EQ(a.x, b.x);
	EXPECT_FLOAT_EQ(a.y, b.y);
	EXPECT_FLOAT_EQ(a.z, b.z);
}
}

TEST(BodyPalette, EveryColorComponentIsInUnitRange)
{
	const BodyPalette palette;
	for (std::size_t i = 0; i < BODY_PART_COUNT; ++i)
	{
		const Vec3& c = palette.color(partAt(i));
		for (float v : {c.x, c.y, c.z})
		{
			EXPECT_GE(v, 0.0f) << bodyPartName(partAt(i));
			EXPECT_LE(v, 1.0f) << bodyPartName(partAt(i));
		}
	}
}

TEST(BodyPalette, LeftAndRightPartsShareTheSameColor)
{
	const BodyPalette p;
	expectSameColor(p.color(BodyPart::UpperArmLeft), p.color(BodyPart::UpperArmRight));
	expectSameColor(p.color(BodyPart::ForearmLeft), p.color(BodyPart::ForearmRight));
	expectSameColor(p.color(BodyPart::HandLeft), p.color(BodyPart::HandRight));
	expectSameColor(p.color(BodyPart::ThighLeft), p.color(BodyPart::ThighRight));
	expectSameColor(p.color(BodyPart::LowerLegLeft), p.color(BodyPart::LowerLegRight));
	expectSameColor(p.color(BodyPart::FootLeft), p.color(BodyPart::FootRight));
}

TEST(BodyPalette, VisibleSkinPartsShareTheSkinColor)
{
	const BodyPalette p;
	expectSameColor(p.color(BodyPart::Head), p.color(BodyPart::Neck));
	expectSameColor(p.color(BodyPart::Head), p.color(BodyPart::HandLeft));
	expectSameColor(p.color(BodyPart::Head), p.color(BodyPart::ForearmLeft));
}

TEST(BodyPalette, ClothesDifferFromSkin)
{
	const BodyPalette p;
	const Vec3 skin = p.color(BodyPart::Head);
	EXPECT_GT((p.color(BodyPart::Torso) - skin).length(), 0.05f);
	EXPECT_GT((p.color(BodyPart::ThighLeft) - skin).length(), 0.05f);
	EXPECT_GT((p.color(BodyPart::FootLeft) - skin).length(), 0.05f);
}

TEST(BodyPalette, SetColorChangesOnlyThatPart)
{
	BodyPalette p;
	const Vec3 other = p.color(BodyPart::Head);
	p.setColor(BodyPart::Torso, {1.0f, 0.0f, 0.5f});
	expectSameColor(p.color(BodyPart::Torso), {1.0f, 0.0f, 0.5f});
	expectSameColor(p.color(BodyPart::Head), other);
}
