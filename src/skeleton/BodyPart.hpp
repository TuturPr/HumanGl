#pragma once

#include <array>
#include <cstddef>

// Every drawable part of the character. The first ten are required by the
// subject; Neck, hands and feet are bonus parts that can be switched off.
enum class BodyPart
{
	Torso,
	Head,
	UpperArmLeft,
	ForearmLeft,
	UpperArmRight,
	ForearmRight,
	ThighLeft,
	LowerLegLeft,
	ThighRight,
	LowerLegRight,
	Neck,
	HandLeft,
	HandRight,
	FootLeft,
	FootRight,
	Count
};

constexpr std::size_t BODY_PART_COUNT = static_cast<std::size_t>(BodyPart::Count);

constexpr std::size_t index(BodyPart part)
{
	return static_cast<std::size_t>(part);
}

constexpr bool isBonusPart(BodyPart part)
{
	return index(part) >= index(BodyPart::Neck);
}

// Left limbs sit on +X (the character faces +Z), right limbs on -X.
enum class Side
{
	Left,
	Right
};

constexpr float sideSign(Side side)
{
	return side == Side::Left ? 1.0f : -1.0f;
}

const char* bodyPartName(BodyPart part);

// The same part on the other side of the body (itself for Torso, Head, Neck).
BodyPart mirrorPart(BodyPart part);
