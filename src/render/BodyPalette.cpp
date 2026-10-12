#include "render/BodyPalette.hpp"

namespace {

constexpr Vec3 SKIN   = {0.87f, 0.66f, 0.52f};
constexpr Vec3 SHIRT  = {0.93f, 0.93f, 0.90f};
constexpr Vec3 SLEEVE = {0.85f, 0.85f, 0.82f};
constexpr Vec3 PANTS  = {0.16f, 0.20f, 0.33f};
constexpr Vec3 SHOES  = {0.12f, 0.10f, 0.09f};

} // namespace

BodyPalette::BodyPalette()
{
	colors_[index(BodyPart::Torso)]         = SHIRT;
	colors_[index(BodyPart::Head)]          = SKIN;
	colors_[index(BodyPart::Neck)]          = SKIN;
	colors_[index(BodyPart::UpperArmLeft)]  = SLEEVE;
	colors_[index(BodyPart::UpperArmRight)] = SLEEVE;
	colors_[index(BodyPart::ForearmLeft)]   = SKIN;
	colors_[index(BodyPart::ForearmRight)]  = SKIN;
	colors_[index(BodyPart::HandLeft)]      = SKIN;
	colors_[index(BodyPart::HandRight)]     = SKIN;
	colors_[index(BodyPart::ThighLeft)]     = PANTS;
	colors_[index(BodyPart::ThighRight)]    = PANTS;
	colors_[index(BodyPart::LowerLegLeft)]  = PANTS;
	colors_[index(BodyPart::LowerLegRight)] = PANTS;
	colors_[index(BodyPart::FootLeft)]      = SHOES;
	colors_[index(BodyPart::FootRight)]     = SHOES;
}

const Vec3& BodyPalette::color(BodyPart part) const
{
	return colors_[index(part)];
}

void BodyPalette::setColor(BodyPart part, const Vec3& color)
{
	colors_[index(part)] = color;
}
