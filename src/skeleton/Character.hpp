#pragma once

#include "skeleton/HumanModel.hpp"

#include <vector>

// A HumanModel standing on the floor (y = 0). Before drawing, the model is
// traversed once to find where it touches the ground:
//  - it is lifted so that its lowest point rests on the floor, so bent knees,
//    resized legs or hands on the floor stay in contact with the ground
//    without the animations knowing any dimension;
//  - if the pose asks for it (supportPinning), it is also shifted
//    horizontally so that the contact point stays under rootOffset.x / z.
class Character
{
public:
	// `contactHull` are points of the unit shape used to find the contact;
	// by default the 8 corners of the unit cube.
	Character();
	explicit Character(std::vector<Vec3> contactHull);

	HumanModel& model();
	const HumanModel& model() const;

	struct Contact
	{
		float lowest;	// height of the lowest point
		Vec3 support;	// average of the points touching the floor
	};
	// Contact of the body when drawn with pose.rootOffset = 0.
	Contact contact(const Pose& pose) const;
	void draw(MatrixStack& stack, const Pose& pose, IPartDrawer& drawer) const;

	// Points less than this above the lowest point count as touching the floor.
	static constexpr float CONTACT_TOLERANCE = 0.03f;

private:
	HumanModel model_;
	std::vector<Vec3> contactHull_;
};
