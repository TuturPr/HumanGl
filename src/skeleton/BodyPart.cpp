#include "skeleton/BodyPart.hpp"

const char* bodyPartName(BodyPart part)
{
	static constexpr std::array<const char*, BODY_PART_COUNT> names = {
		"Torso", "Head",
		"Upper arm L", "Forearm L", "Upper arm R", "Forearm R",
		"Thigh L", "Lower leg L", "Thigh R", "Lower leg R",
		"Neck", "Hand L", "Hand R", "Foot L", "Foot R",
	};
	return part < BodyPart::Count ? names[index(part)] : "Unknown";
}

BodyPart mirrorPart(BodyPart part)
{
	switch (part)
	{
		case BodyPart::UpperArmLeft:  return BodyPart::UpperArmRight;
		case BodyPart::UpperArmRight: return BodyPart::UpperArmLeft;
		case BodyPart::ForearmLeft:   return BodyPart::ForearmRight;
		case BodyPart::ForearmRight:  return BodyPart::ForearmLeft;
		case BodyPart::HandLeft:      return BodyPart::HandRight;
		case BodyPart::HandRight:     return BodyPart::HandLeft;
		case BodyPart::ThighLeft:     return BodyPart::ThighRight;
		case BodyPart::ThighRight:    return BodyPart::ThighLeft;
		case BodyPart::LowerLegLeft:  return BodyPart::LowerLegRight;
		case BodyPart::LowerLegRight: return BodyPart::LowerLegLeft;
		case BodyPart::FootLeft:      return BodyPart::FootRight;
		case BodyPart::FootRight:     return BodyPart::FootLeft;
		default:                      return part;
	}
}
