#pragma once

#include "skeleton/HumanModel.hpp"

#include <vector>

// A HumanModel standing on the floor (y = 0). Before drawing, the model is
// traversed once to find its lowest point and lifted so that this point rests
// on the ground. Bent knees, resized legs or hands on the floor therefore stay
// in contact with the ground without the animations knowing any dimension.
class Character
{
public:
	// `contactHull` are points of the unit shape used to find the lowest
	// point; by default the 8 corners of the unit cube.
	Character();
	explicit Character(std::vector<Vec3> contactHull);

	HumanModel& model();
	const HumanModel& model() const;

	// Height of the lowest point of the body when drawn with pose.rootOffset.y = 0.
	float lowestPoint(const Pose& pose) const;
	void draw(MatrixStack& stack, const Pose& pose, IPartDrawer& drawer) const;

private:
	HumanModel model_;
	std::vector<Vec3> contactHull_;
};
