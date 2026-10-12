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
