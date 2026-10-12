#include "skeleton/Character.hpp"

#include <algorithm>
#include <limits>
#include <utility>

namespace {

std::vector<Vec3> unitCubeCorners()
{
	std::vector<Vec3> corners;
	for (float x : {-0.5f, 0.5f})
		for (float y : {-0.5f, 0.5f})
			for (float z : {-0.5f, 0.5f})
				corners.push_back({x, y, z});
	return corners;
}

// Part drawer that only records the lowest world-space point of the body.
class LowestPointFinder : public IPartDrawer
{
public:
	explicit LowestPointFinder(const std::vector<Vec3>& hull) : hull_(hull) {}

	void drawUnitShape(BodyPart, const Mat4& model) override
	{
		for (const Vec3& point : hull_)
			lowest_ = std::min(lowest_, model.transformPoint(point).y);
	}

	float lowest() const { return lowest_; }

private:
	const std::vector<Vec3>& hull_;
	float lowest_ = std::numeric_limits<float>::max();
};

Pose withoutLift(const Pose& pose)
{
	Pose grounded = pose;
	grounded.rootOffset.y = 0.0f;
	return grounded;
}

} // namespace

Character::Character() : contactHull_(unitCubeCorners()) {}

Character::Character(std::vector<Vec3> contactHull) : contactHull_(std::move(contactHull)) {}

HumanModel& Character::model()
{
	return model_;
}

const HumanModel& Character::model() const
{
	return model_;
}

float Character::lowestPoint(const Pose& pose) const
{
	MatrixStack stack;
	LowestPointFinder finder(contactHull_);
	model_.draw(stack, withoutLift(pose), finder);
	return finder.lowest();
}

void Character::draw(MatrixStack& stack, const Pose& pose, IPartDrawer& drawer) const
{
	const Pose grounded = withoutLift(pose);

	stack.push();
	stack.translate({0.0f, pose.rootOffset.y - lowestPoint(grounded), 0.0f});
	model_.draw(stack, grounded, drawer);
	stack.pop();
}
