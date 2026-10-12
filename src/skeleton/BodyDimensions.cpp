#include "skeleton/BodyDimensions.hpp"

#include <algorithm>

BodyDimensions::BodyDimensions()
{
	sizes_[index(BodyPart::Torso)]         = {0.40f, 0.60f, 0.22f};
	sizes_[index(BodyPart::Head)]          = {0.22f, 0.26f, 0.24f};
	sizes_[index(BodyPart::UpperArmLeft)]  = {0.11f, 0.30f, 0.11f};
	sizes_[index(BodyPart::ForearmLeft)]   = {0.09f, 0.27f, 0.09f};
	sizes_[index(BodyPart::ThighLeft)]     = {0.15f, 0.44f, 0.16f};
	sizes_[index(BodyPart::LowerLegLeft)]  = {0.11f, 0.42f, 0.11f};
	sizes_[index(BodyPart::Neck)]          = {0.10f, 0.08f, 0.10f};
	sizes_[index(BodyPart::HandLeft)]      = {0.09f, 0.17f, 0.04f};
	sizes_[index(BodyPart::FootLeft)]      = {0.10f, 0.07f, 0.25f};

	// Right side mirrors the left side.
	sizes_[index(BodyPart::UpperArmRight)] = sizes_[index(BodyPart::UpperArmLeft)];
	sizes_[index(BodyPart::ForearmRight)]  = sizes_[index(BodyPart::ForearmLeft)];
	sizes_[index(BodyPart::ThighRight)]    = sizes_[index(BodyPart::ThighLeft)];
	sizes_[index(BodyPart::LowerLegRight)] = sizes_[index(BodyPart::LowerLegLeft)];
	sizes_[index(BodyPart::HandRight)]     = sizes_[index(BodyPart::HandLeft)];
	sizes_[index(BodyPart::FootRight)]     = sizes_[index(BodyPart::FootLeft)];
}

const Vec3& BodyDimensions::size(BodyPart part) const
{
	return sizes_[index(part)];
}

void BodyDimensions::setSize(BodyPart part, const Vec3& size)
{
	sizes_[index(part)] = {
		std::max(size.x, MIN_SIZE),
		std::max(size.y, MIN_SIZE),
		std::max(size.z, MIN_SIZE),
	};
}
